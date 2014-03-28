#include "jimk.h"
#include "auto.h"
#include "broadcas.h"
#include "celmenu.h"
#include "errcodes.h"
#include "flicel.h"
#include "flx.h"
#include "inks.h"
#include "palmenu.h"
#include "pentools.h"
#include "rastcurs.h"
#include "rastrans.h"
#include "render.h"
#include "softmenu.h"
#include "util.h"

static void kill_overlay(Button *b);
static void render_overlay(Button *b);
static void merge_overlay(Button *b);
static void free_paint_buffers(void);
static Errcode alloc_paint_buffers(void);

static Button apa_paintopt_sel = MB_INIT1(
	NONEXT, /* next */
	&cmu_pa_group_sel,
	0, 0, 121, 27, /* w,h,x,y */
	NOTEXT,
	hang_children, 	  /* seeme */
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);
static Button apa_rendseq_sel = MB_INIT1(
	&apa_paintopt_sel,
	NOCHILD, /* children */
	108, 9, 6, 29, /* w,h,x,y */
	NODATA, /* "Render Sprite", */
	ccorner_text,
	render_overlay,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);
static Button apa_kill_sel = MB_INIT1(
	&apa_rendseq_sel,		/* next */
	NOCHILD, /* children */
	108, 9, 6, 41, /* w,h,x,y */
	NODATA, /* "Cancel Sprite", */
	ccorner_text,
	kill_overlay,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);

static Button apa_title_sel = MB_INIT1(
	&apa_kill_sel, 	/* next */
	&cmu_common_sels, 		/* children */
	110, 9, 5, 3, 	/* w,h,x,y */
	NODATA, /* "Sprite", */
	hang_see_title,
	mb_clipmove_menu,
	mb_menu_to_bottom,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);

static Menuhdr anipaste_menu = MENU_INIT0(
	320,70,0,0,   /* width, height, x, y */
	ANIPASTE_MUID,	/* id */
	PANELMENU,		/* type */
	&apa_title_sel,	/* buttons */
	SCREEN_FONT,	/* font */
	&menu_cursor.hdr,	/* cursor */
	seebg_white, 		/* seebg */
	NULL,					/* dodata */
	NULL,					/* domenu */
	(MBPEN|MBRIGHT|KEYHIT), /* ioflags */
	0,		/* flags */
	NULL,	/* procmouse */
	NULL, 	/* on_showhide */
	NULL	/* cleanup */
);

static Smu_button_list apa_smblist[] =  {
	{ "title",  { &apa_title_sel } },
	{ "rend",   { &apa_rendseq_sel } },
	{ "kill",   { &apa_kill_sel } },
};

static Boolean do_pastemenu_keys(void)
{
 	if(check_toggle_menu()
		|| common_header_keys()
		|| check_undo_key())
	{
		return(TRUE);
	}
	return(FALSE);
}

static Errcode go_paste_menu(void)
/* it is assumed undo is valid under cel and cel is not present
 * when this is called */
{
Errcode err;
void *ss;
VFUNC oundo;

	if(MENU_ISOPEN(&anipaste_menu)) /* to prevent recursion */
		return(Success);

	if((err = soft_buttons("sprite_panel", apa_smblist, Array_els(apa_smblist),
						   &ss)) < Success)
	{
		return(err);
	}
	oundo = vl.undoit;
	vl.undoit = NULL;

	fliborder_on();
	cm_restore_toolcel();
	enable_toolcel_redraw();

	disable_palette_menu();
	menu_to_point(vb.screen,&anipaste_menu,cel_menu.x + (cel_menu.width>>1),
										   cel_menu.y + (cel_menu.height>>1));
	err = do_menuloop(vb.screen,&anipaste_menu,NULL,NULL,do_pastemenu_keys);

	enable_palette_menu();
	vl.undoit = oundo;
	smu_free_scatters(&ss);
	return(err);
}
/******************************************************************/
static Errcode apa_realloc_overlays(void)
{
Errcode err;

	cmcb->olaymem = 0;
	cmcb->num_overlays = 0;
	cmcb->outta_mem = FALSE;
	if((err = alloc_flx_olaytab(&flix, flix.hdr.frames_in_table)) < Success)
		cmcb->outta_mem = TRUE;
	return(err);
}
static void kill_overlay(Button *b)

/* note toolcel is not restored that this is done on exit of paint tool loop 
 * when menu is closed */ 
{
	unzoom();
	mb_gclose_code(b,Err_abort);
	apa_realloc_overlays();
	cm_erase_toolcel();
	soft_put_wait_box("celseq_del");
	restore();
	save_undo();
	cleanup_wait_box();
	rezoom();
}
static Errcode rend_olay(Flx_overlay *olay)
{
Errcode err;

	if(!(olay->flags & FOVL_CEL))
		return(Success);

	vs.render_under = ((olay->flags & FOVL_UNDER) != 0);
	vs.zero_clear = ((olay->flags & FOVL_ZCLEAR) != 0);
	vs.fit_colors = ((olay->flags & FOVL_CFIT) != 0);
	vs.render_one_color = ((olay->flags & FOVL_ONECOL) != 0);
	vs.ccolor = olay->ccolor;
	vs.inks[0] = olay->ink0;
	thecel->cd.cent = olay->cpos;
	seek_fcel_frame(thecel,olay->cframe);

	if((err = draw_flicel(thecel,DRAW_RENDER,NEW_CFIT)) < 0)
		return(err);

	if(vl.ink && (vl.ink->needs & INK_NEEDS_UNDO))
		save_fcel_undo(thecel);
}

static Errcode
render1_ram_olay(void *poverlays, int ix, int intween, int scale, Autoarg *aa)
/* only to be called with dall so aa->cur_frame is the frame ix */
{
	Flx_overlay **overlays = poverlays;
	Flx_overlay *olay;
	Errcode err = Success;
	(void)ix;
	(void)intween;
	(void)scale;

	start_abort_atom();
	if(aa->cur_frame == 0)
		olay = overlays[flix.hdr.frame_count]; /* in the ring record */
	else
		olay = overlays[aa->cur_frame]; 

	while(olay)
	{
		if((err = rend_olay(olay)) < 0)
			goto error;
		olay = olay->next;
	}
error:
	return(errend_abort_atom(err));
}

typedef struct fileolay_dat {
	Jfile fd;
	LONG size_left;	/* file to go */
	LONG next_pos;  /* position of next chunk */
	Chunk_id next_one;
} Fod;

static Errcode
render1_file_olay(void *data, int ix, int intween, int scale, Autoarg *aa)
{
	Fod *fod = data;
	Errcode err;
	Flx_overlay solay; /* stack overlay */
	(void)ix;
	(void)intween;
	(void)scale;

	start_abort_atom();
	if(aa->cur_frame == 0)
	{
		if(JNONE == (fod->fd = pj_open(flxolayname,JREADONLY)))
		{
			err = pj_ioerr();
			goto error;
		}
		if((err = pj_read_ecode(fod->fd,&fod->next_one,sizeof(Chunk_id))) < 0)
			goto error;
		fod->size_left = fod->next_one.size - sizeof(Chunk_id);
		fod->next_pos = sizeof(Chunk_id);
		goto get_next_chunk; /* only first time */
	}

	err = Success;

	if(fod->size_left <= 0)
		goto out;


	while(fod->next_one.type <= aa->cur_frame) 
	{
		if((err = pj_read_ecode(fod->fd,&solay,POSTOSET(Flx_overlay,cpos))) < 0)
			break;

		fod->size_left -= fod->next_one.size;

		solay.next = NULL;
		if((err = rend_olay(&solay)) < 0)
			break;

get_next_chunk:
		if(fod->size_left <= 0)
		{
			pj_close(fod->fd);
			fod->fd = 0;
			break;
		}

		if((err = pj_readoset(fod->fd,&fod->next_one,
							 fod->next_pos,sizeof(Chunk_id))) < 0)
		{
			break;
		}
		fod->next_pos += fod->next_one.size;
	}

error:
out:
	return(errend_abort_atom(err));
}
static void render_overlay(Button *b)
{
Errcode err;
void *cbuf;
Rcel *rc;
Autoarg aa;
Boolean ovpushed = 0;
Fcelpos opos;
SHORT ounder;
SHORT ozclear;
SHORT ocfit;
SHORT o1color;
Pixel occolor;
Pixel oink0;
Fod fod;
void *oovlays;
SHORT oocount;

	fod.fd = 0;

	if(vl.ink->ot.id == opq_INKID && !(vs.make_mask || vs.use_mask))
	{
		merge_overlay(b);
		return;
	}

	get_fcelpos(thecel,&opos);
	cm_erase_toolcel();

	unzoom();
	hide_mp();
	pj_delete(bscreen_name);
	push_inks(); /* push alt if we can */

	/* the auto dall() function needs an additional screen and a
	 * cbuf for the screen so we try here to decide to free overlay
	 * records and use a file or not */

	clear_struct(&aa);

	if((err = pj_fli_cel_alloc_cbuf(&cbuf,vb.pencel)) >= Success)
	{
		err = alloc_pencel(&rc);
		pj_rcel_free(rc);
		pj_free(cbuf);
	}

	if(err < 0) /* need some mem use a temp file */
	{
		soft_put_wait_box("celseq_todisk");
		if((err = push_flx_overlays()) < 0)
			goto error;
		ovpushed = 1;
		aa.avec = render1_file_olay;
		aa.avecdat = &fod;
	}
	else
	{
		aa.avec = render1_ram_olay;
		aa.avecdat = flix.overlays;
	}
	aa.flags = AUTO_USESCEL|AUTO_NOCELSEEK;

	if(vl.ink && (vl.ink->needs & INK_NEEDS_UNDO))
		aa.flags |= AUTO_USES_UNDO;

	free_paint_buffers();

	occolor = vs.ccolor;
	ounder = vs.render_under;
	ozclear = vs.zero_clear;
	o1color = vs.render_one_color;
	ocfit = vs.fit_colors;
	oink0 = vs.inks[0];

	oovlays = flix.overlays;    /* to prevent rendering of overlays in auto */
	oocount = flix.overlays_in_table;
	flix.overlays = NULL;
	flix.overlays_in_table = 0;

	err = noask_do_auto(&aa,DOAUTO_ALL);

	flix.overlays_in_table = oocount;
	flix.overlays = oovlays;

	vs.inks[0] = oink0;
	vs.render_under = ounder; 
	vs.zero_clear = ozclear;
	vs.render_one_color = o1color;
	vs.fit_colors = ocfit;
	vs.ccolor = occolor;

	alloc_paint_buffers();

	if(err < Success)
		goto error;

	if(ovpushed)
		pj_delete(flxolayname);

	pj_close(fod.fd);
	if(thecel)
		put_fcelpos(thecel,&opos);
	apa_realloc_overlays();
	pop_inks();
	rezoom();
	show_mp();
	mb_gclose_code(b,Err_abort);
	return;
error:
	pj_close(fod.fd);
	if(thecel)
		put_fcelpos(thecel,&opos);
	pop_inks();
	rezoom();
	show_mp();
	if(ovpushed)
	{
		if((err = pop_flx_overlays()) < Success)
		{
			softerr(err,"seq_lost");	
			kill_overlay(b);
			return;
		}
	}
	restore_with_overlays();
	cm_restore_toolcel();
	return;
}
static void merge_overlay(Button *b)

/* note toolcel is not restored that this is done on exit of paint tool loop 
 * when menu is closed */ 
{
Errcode err;

	hide_mp();
	cm_erase_toolcel();
	fake_push_cel();

	free_paint_buffers();
	err = auto_merge_overlays();

	alloc_paint_buffers();
	if(err >= Success)
	{
		apa_realloc_overlays();
		mb_gclose_code(b,Err_abort);
	}
	else
		show_mp();

	fake_pop_cel();
}

static Errcode draw_onecolor_fcel(Flicel *fc, Pixel color)
{
Errcode err;
BYTE o1col;
SHORT occolor;
Celcfit *ocfit;
Celcfit cfit;
(void)fc;

	/* save old render state */
	o1col = vs.render_one_color;
	occolor = vs.ccolor;
	ocfit = thecel->cfit;

	vs.render_one_color = 1;
	vs.ccolor = color;

	init_celcfit(&cfit);
	make_render_cfit(thecel->rc->cmap,&cfit,thecel->cd.tcolor);

	thecel->cfit = &cfit;
	err = draw_flicel(thecel,DRAW_FIRST,OLD_CFIT);

	vs.render_one_color = o1col;
	thecel->cfit = ocfit;
	vs.ccolor = occolor;
	return(err);
}
static void draw_blue_cel(void)
{
	draw_onecolor_fcel(thecel,vs.inks[1]);
}

static void do_delta_celcurs(SHORT dx,SHORT dy)
{
	translate_flicel(thecel,dx,dy);
	maybe_ref_flicel_pos(thecel);
	/* save new part to be covered by cel */
	do_leftbehind(thecel->xf.mmax.x,thecel->xf.mmax.y,
				  thecel->xf.ommax.x,thecel->xf.ommax.y,
				  thecel->xf.ommax.width,thecel->xf.ommax.height,
				  (do_leftbehind_func)save_undo_rect );

	if(vs.cycle_draw && vs.render_one_color)
		draw_flicel(thecel,DRAW_DELTA,NEW_CFIT);
	else
		draw_flicel(thecel,DRAW_DELTA,OLD_CFIT);
}
static Errcode advance_draw_fli(Fli_frame *frame, 
								VFUNC pre_decomp, void *pddat)
/* increment fli for pasting wrapping through ring frame if appropriate */
{
SHORT oix;
Errcode err;

	oix = vs.frame_ix;
	if(!flix.fd)
	{
		err = Err_not_found;
		goto error;
	}

	if(++vs.frame_ix > flix.hdr.frame_count)
		check_loop();

	if((err = read_flx_frame(&flix, frame, vs.frame_ix)) < 0)
		goto error;

	if(pre_decomp != NULL)
		(*(pre_decomp))(pddat);

	pj_fli_uncomp_frame(vb.pencel,frame,1);
	unfli_flx_overlay(&flix,vb.pencel,vs.frame_ix);

	if(vs.frame_ix == flix.hdr.frame_count)
		vs.frame_ix = 0;
	zoom_it();
	return(Success);
error:
	vs.frame_ix = oix;
	pj_gentle_free(frame);
	return(softerr(err,"fli_frame"));
}
/*************** paste by building and adding overlays ************/

static Errcode add_cel_overlay(Fli_frame *cbuf, Short_xy *cpos, SHORT frame,
							Rectangle *cr,int frame_ix)
{
Errcode err;
LONG size;
Rectangle ovrect;
extern long pj_mem_used;


	if(!flix.overlays) /* protection */
		return(Err_bad_input);

	ovrect.x = ovrect.y = 0;
	ovrect.width = vb.pencel->width;
	ovrect.height = vb.pencel->height;

	if(!(and_rects(cr,&ovrect,&ovrect))) /* if clipped out don't bother */
		return(Success);

	if((size = pj_fli_comp_rect(cbuf, undof, vb.pencel, 
						 &ovrect, 0, COMP_DELTA_FRAME,pj_fli_comp_ani)) < 0)
	{
		err = size;
		goto error;
	}
	size = pj_mem_used;
	if((err = add_flx_olayrec(cpos, frame, &ovrect, cbuf, frame_ix)) < 0)
		goto error;

	size = pj_mem_used - size;
	cmcb->olaymem += size;
	++cmcb->num_overlays;
	vs.bframe_ix = 0;


#ifdef TESTING
	/* Possibly put a test for how much you have left and put up a warning */

	if(debug)
		top_textf("tot mem %d comp size %d", cmcb->olaymem, size);
#endif

	err = Success;
error:
	return(err);
}
static void save_not_within(Rectangle *chgr, Rectangle *celr)

/* saves area in changes rect not within cel rect or the last cel rect 
 * to undof. this clips */
{
SHORT x,y,x1,y1,width,height;

#ifdef DEBUG
 #define save_undo_rect(x,y,w,h) pj_set_rect(vb.pencel,sblack,x,y,w,h)

	pj_clear_rast(vb.pencel);
	pj_clear_rast(undof);
#endif


	if(chgr->height > (celr->height<<1))
	{
		/* do horiz rect below top one and above bottom one */

		save_undo_rect(chgr->x,chgr->y + celr->height,
					   chgr->width,
					   chgr->height - (celr->height<<1));

		height = celr->height;
	}
	else /* vertical overlap */
	{
		if(chgr->width > (celr->width<<1)) /* horiz separation */
		{
			/* do area between left and right one in vertical overlap */

			width = chgr->width - (celr->width<<1);
			height = (celr->height<<1) - chgr->height;
			save_undo_rect(chgr->x + celr->width,
						   chgr->y + celr->height - height,
						   width, height);
		}
		height = chgr->height - celr->height;
	}

	width = chgr->width - celr->width;

	if((x = celr->x) == chgr->x)
	{
		x1 = x;
		x += celr->width;
	}
	else
	{
		x1 = chgr->x + celr->width;
		x = chgr->x;
	}
	
	if((y = celr->y) == chgr->y)
	{
		y1 = y + chgr->height - height; 
	}
	else
	{
		y1 = chgr->y;
		y += (celr->height - height);
	}

	save_undo_rect(x,y,width,height);
	save_undo_rect(x1,y1,width,height);
}

static void restore_from_rast2(Celmu_cb *cb)
{
	pj_blitrect(cb->crast2,0,0,
		 	 vb.pencel,thecel->xf.mmax.x,thecel->xf.mmax.y,
		 	 thecel->xf.mmax.width,thecel->xf.mmax.height);
}
static Errcode finish_cel_overlay(void)
{
Errcode err;
Rectangle changerect;
Raster *rast1;
Raster *rast2;
SHORT frame_ix;
Fli_frame *cbuf;

	if(NULL == (cbuf = pj_malloc(cmcb->finish_buf_size)))
	{
		cmcb->outta_mem = TRUE;
		return(Err_no_memory);
	}

	rast1 = cmcb->crast1;
	changerect.x = rast1->x;
	changerect.y = rast1->y;
	changerect.width = thecel->xf.mmax.width;
	changerect.height = thecel->xf.mmax.height;

	if(vs.cm_blue_last && !vs.cm_streamdraw)
	{
		rast2 = cmcb->crast2;

		/* replace blued cel in undo */
		pj_blitrect(rast2,0,0,undof,rast2->x,rast2->y,changerect.width,
												   changerect.height);
		/* replace blued cel in pencel */
		pj_blitrect(rast2,0,0,vb.pencel,rast2->x,rast2->y,changerect.width,
												       changerect.height);
	}

	/* remove cel from current frame */
	unsee_flicel(thecel);

	pj_blitrect(rast1,0,0,
		 	 undof,rast1->x,rast1->y,
		 	 thecel->xf.mmax.width,thecel->xf.mmax.height);

	frame_ix = vs.frame_ix!=0?vs.frame_ix:flix.hdr.frame_count;

	/* unfli in undo on top of cel and previous frame */
	gb_unfli(undof,frame_ix,0,cbuf);

	/* build and add new overlay for current frame without cel data */
	err = add_cel_overlay(cbuf, NULL, 0, &changerect,frame_ix);
	pj_free(cbuf);

	/* restore undo behind cel */
	save_fcel_undo(thecel);
	cmcb->undo_corrupted = TRUE;
	return(err);
}
static Errcode paste_cel_overlay(Boolean delta)

/* build overlay record to add to current frame and increment fli 
 * leave next frame without cel present assumes undof has current or previous
 * frame image under previous cel's position and current image under
 * cel's current position */
{
Errcode err;
Rectangle changerect;
SHORT dx, dy;
Raster *rast1;
Raster *rast2;
SHORT width, height;
int frame_ix;
Boolean blue_last;
Fli_frame *cbuf = NULL;

	copy_rectfields(&thecel->xf.mmax,&changerect); /* get current position */
	width = changerect.width;
	height = changerect.height;
	rast1 = cmcb->crast1;
	rast2 = cmcb->crast2;
	blue_last = (vs.cm_blue_last && !vs.cm_streamdraw);

	if(delta)
	{
		dx = changerect.x - cmcb->crast1->x;
		if(dx > 0)
			changerect.x -= dx;
		else
			dx = -dx;

		changerect.width += dx;

		dy = changerect.y - cmcb->crast1->y;

		if(dy > 0)
			changerect.y -= dy;
		else
			dy = -dy;

		changerect.height += dy;

		save_not_within(&changerect, 
						(Rectangle *)&(thecel->xf.mmax.RECTSTART));

		if(blue_last)
		{
			/* replace blued cel in undo */
			pj_blitrect(rast2,0,0,undof,rast2->x,rast2->y,width,height);
			/* replace blued cel in pencel */
			pj_blitrect(rast2,0,0,vb.pencel,rast2->x,rast2->y,width,height);
			/* redraw cel in case obliterated by un-blue */
			draw_flicel(thecel,DRAW_FIRST,OLD_CFIT); /* draw new cel */
		}
	}

	/* save undo area behind current cel in rast2 */

	pj_blitrect(undof,thecel->xf.mmax.x,thecel->xf.mmax.y,
			 rast2,0,0,width,height);

	if(delta)
	{
		/* put previous cel's rendered rectangle in undo at it's position */
		pj_blitrect(rast1,0,0, undof,rast1->x,rast1->y, width,height);
	}

	frame_ix = vs.frame_ix!=0?vs.frame_ix:flix.hdr.frame_count;

	if((err = ealloc(&cbuf,cmcb->olay_buf_size)) < Success)
		goto restore_error;

	/* unfli in undo on top of cel and previous frame */
	if((err = gb_unfli(undof,frame_ix,0,cbuf)) < 0)
		goto restore_error;

	/* build and add new overlay for current frame with current cel data */

	if((err = add_cel_overlay(cbuf,&thecel->cd.cent, thecel->cd.cur_frame, 
							  &changerect, frame_ix)) < 0)
	{
		goto restore_error;
	}

	/* save current cel's rendered image on current screen in rast1 */

	rast1->x = thecel->xf.mmax.x;
	rast1->y = thecel->xf.mmax.y;

	pj_blitrect(vb.pencel,rast1->x,rast1->y,rast1,0,0,width,height);

	/* erase cel from saved buffer #2 at current cel's position
	 * and advanvce fli */

	advance_draw_fli(cbuf,restore_from_rast2,cmcb);
	pj_freez(&cbuf);

	if(blue_last)
	{
		/* save area under "blue" in rast2 */
		rast2->x = thecel->xf.mmax.x;
		rast2->y = thecel->xf.mmax.y;
		pj_blitrect(vb.pencel,rast2->x,rast2->y,rast2,0,0,width,height);
		/* draw new blue cel at current position */
		draw_blue_cel();
	}

	/* certainly is */
	cmcb->undo_corrupted = TRUE;
	return(Success);

restore_error:

	pj_freez(&cbuf);

	/* restore undo area under cel */

	pj_blitrect(rast2,0,0,undof,
			 thecel->xf.mmax.x,thecel->xf.mmax.y, width,height);

	/* get out of here */
	if(err == Err_no_memory)
		cmcb->outta_mem = TRUE;
	return(err);
}


static void free_fcel_backup(Raster **bup)
{
	pj_rast_free(*bup);
	*bup = NULL;
}
static Errcode realloc_fcel_backup(Flicel *fc, Raster **bup)
{
	free_fcel_backup(bup);
	refresh_flicel_pos(fc); /* update mmax */

#ifdef DEBUG
	{
	extern char *rex_name;

		if(!txtcmp(pj_get_path_name(rex_name),"moo.drv"))
		{
			/* for debugging get off card */
			if (vd_open_cel(vb.vd, bup, fc->xf.mmax.width,
										fc->xf.mmax.height, FALSE) < 0)
			{
			return(valloc_bytemap(bup,fc->xf.mmax.width,fc->xf.mmax.height));
			}
			return(Success);
		}
	}
#endif

	return(valloc_bytemap(bup,fc->xf.mmax.width,fc->xf.mmax.height));
}

static void free_paint_buffers(void)
{
	free_fcel_backup(&cmcb->crast1);
	free_fcel_backup(&cmcb->crast2);
}
static Errcode alloc_paint_buffers(void)

/* alloc buffers needed exclusively for paint tool */
{
Errcode err;
LONG size;
void *cbuf;

	/* make size big enough so that if we can't get it we have enough to
	 * recover */

	cmcb->finish_buf_size = pj_fli_cbuf_size(vb.pencel->width,vb.pencel->height,
			 				 		vb.pencel->cmap->num_colors);

	size = fcel_cbuf_size(thecel);
	if(!fcel_needs_seekbuf(thecel))
		size = Min(32000,size); 
		
	cmcb->olay_buf_size = cmcb->finish_buf_size + size;

	if((err = realloc_fcel_backup(thecel, &cmcb->crast1)) < 0)
		goto error;
	if((err = realloc_fcel_backup(thecel, &cmcb->crast2)) < 0)
		goto error;

	/* se if we have enough for the buffer */

	if((cbuf = pj_malloc(cmcb->olay_buf_size)) == NULL)
	{
		err = Err_no_memory;
		goto error;
	}
	else
		pj_free(cbuf);

	return(Success);
error:
	free_paint_buffers();
	return(err);
}
static Errcode insure_paint_buffers(void)
/* Make sure we've got them or else! */
{
	if(cmcb->outta_mem)
		return(Err_no_memory);

	if(    cmcb->crast1 != NULL 
		&& cmcb->crast1->width == thecel->xf.mmax.width
		&& cmcb->crast1->height == thecel->xf.mmax.height
		&& cmcb->crast2 != NULL
		&& cmcb->crast2->width == thecel->xf.mmax.width
		&& cmcb->crast2->height == thecel->xf.mmax.height)
	{
		return(Success);
	}
	return(alloc_paint_buffers());
}
void exit_paint_ctool(Pentool *pt)
{
	free_flx_overlays(&flix);
	free_paint_buffers();
	cmcb->outta_mem = FALSE;

	if(cmcb->undo_corrupted) /* shouldn't happen */
	{
		flx_clear_olays();
		save_undo();
		flx_draw_olays();
	}
	exit_marqi_ctool(pt);
	enable_multi_menu();
}
Errcode init_paint_ctool(Pentool *pt)
{
Errcode err;

	if(!thecel || flix.hdr.frame_count < 2)
		return(Err_abort);

	disable_multi_menu();

	if(need_scrub_frame())
	{
		hide_mp();
		flx_clear_olays();
		scrub_cur_frame();
		save_undo();
		flx_draw_olays();
		show_mp();
	}

	if((err = alloc_paint_buffers()) < 0)
		goto error;

	if((err = apa_realloc_overlays()) < 0)
		goto error;
	if((err = init_marqi_ctool(pt)) < 0)
		return(err);
	return(Success);
error:
	exit_paint_ctool(pt);
	return(err);
}
static void check_cel_ccycle(void)
{
	if(vs.cycle_draw && vs.render_one_color)
	{
		if(vs.cm_streamdraw)
			cycle_ccolor();
		else
			cycle_redraw_ccolor();
	}
}
Errcode cel_paint_ptfunc(Pentool *pt,Wndo *w)
{
Errcode err;
SHORT dx,dy,omx,omy;
Boolean do_delta = FALSE;
LONG clock;
(void)pt;
(void)w;

	cel_cancel_undo();
	if(!thecel)
		return(Success);

	if((err = insure_paint_buffers()) < Success)
		return(softerr(err,"no_sprite"));

	hide_mp();
	if(vs.cm_streamdraw)
		wait_penup();

	disable_toolcel_redraw();
	get_fcelpos(thecel,&cmcb->lastpos);
	cmu_unmarqi_cel();
	draw_flicel(thecel,DRAW_DELTA,OLD_CFIT);
	omx = icb.mx;
	omy = icb.my;

	for(;;)
	{
		if(vs.cm_streamdraw && ISDOWN(MBPEN))
			check_input(MMOVE|KEYHIT|MBPEN|MBPUP|MBRIGHT);
		else
			wait_input(MMOVE|KEYHIT|MBPEN|MBPUP|MBRIGHT);

		dx = icb.mx - omx;
		dy = icb.my - omy;
		omx = icb.mx;
		omy = icb.my;

		if(JSTHIT(MBRIGHT|KEYHIT) && !ISDOWN(MBPEN)) /* user abort */
			break;

		if(JSTHIT(MBPUP))
		{
			if(vs.cm_streamdraw)
			{
				cmcb->undo_corrupted = FALSE;
				unsee_flicel(thecel);
				save_undo();
				translate_flicel(thecel,dx,dy);
				draw_flicel(thecel,DRAW_FIRST,NEW_CFIT);
				continue;
			}
		}
		else if(JSTHIT(MBPEN) || (vs.cm_streamdraw && ISDOWN(MBPEN)))
		{
			if(JSTHIT(MBPEN))
				clock = pj_clock_1000();
			else
			{
				clock += flix.hdr.speed;
				if(!wait_til(clock))
				{
					reuse_input();
					continue;
				}
				if (clock > pj_clock_1000())
					clock = pj_clock_1000();
			}

			if((err = paste_cel_overlay(do_delta)) < Success)
			{
				err = softerr(err,"seq_term");
				break;
			}
			do_delta = TRUE;
			if(vs.paste_inc_cel)
				seek_fcel_frame(thecel, thecel->cd.cur_frame + 1);
			translate_flicel(thecel,dx,dy);
			save_fcel_undo(thecel); /* save undo under new cel */
			check_cel_ccycle();
			draw_flicel(thecel,DRAW_FIRST,NEW_CFIT); /* draw new cel */
			get_fcelpos(thecel,&cmcb->lastpos);
			continue;
		}
		if(JSTHIT(MMOVE))
			do_delta_celcurs(dx,dy);
	}

	if(do_delta)
	{
		err = finish_cel_overlay();
		err = softerr(err,"seq_corrupt");
	}
	else
		unsee_flicel(thecel);

	if(vs.cycle_draw && vs.render_one_color)
		do_color_redraw(NEW_CCOLOR);

	if(cmcb->num_overlays)
	{
		cleanup_toptext();
		put_fcelpos(thecel,&cmcb->lastpos);
		save_fcel_undo(thecel);
		err = go_paste_menu();
	}
	else
		err = Err_abort;

	unsee_flicel(thecel);
	save_undo();
	cmcb->undo_corrupted = FALSE;
	put_fcelpos(thecel,&cmcb->lastpos);
	draw_flicel(thecel,DRAW_FIRST,NEW_CFIT);
	cmu_marqi_cel();
	cleanup_toptext();
	enable_toolcel_redraw();
	show_mp();

	return(err);
}

/******** paste tool ********/
static void cmu_undo_celpaste(void)
{
	if(cmcb->paste_undo != NULL)
	{
		swap_pencels(undof, cmcb->paste_undo);
		zoom_unundo();
		cm_restore_toolcel(); /* wiped out by unundo */
	}
	else
		vl.undoit = NULL;
}
static Errcode save_paste_undo(void)
{
	if(cmcb->paste_undo != NULL)
		pj_rcel_copy(undof,cmcb->paste_undo);
	else
		cmcb->paste_undo = clone_pencel(undof);

	vl.undoit = cmu_undo_celpaste;
	if(cmcb->paste_undo == NULL)
		return(Err_no_memory);
	return(Success);
}
void cmu_free_paste_undo(void)
{
	if(cmcb->paste_undo)
	{
		pj_rcel_free(cmcb->paste_undo);
		cmcb->paste_undo = NULL;
		cel_cancel_undo();
	}
}
void exit_paste_ctool(Pentool *pt)
{
	cmu_free_paste_undo();
	exit_marqi_ctool(pt);
}
Errcode init_paste_ctool(Pentool *pt)
{
Errcode err;
Fli_frame *cbuf;

	if((err = save_paste_undo()) < Success) /* get undo buffer for paste */
		goto error;

	if(thecel) /* make sure we have a cbuf area for cel */
	{		
		if((err = pj_fli_cel_alloc_cbuf(&cbuf,thecel->rc)) < Success)
			goto error;
		pj_free(cbuf);
	}
	if((err = init_marqi_ctool(pt)) < 0)
		goto error;
	return(Success);
error:
	exit_paste_ctool(pt);
	return(err);
}
Errcode cel_paste_ptfunc(Pentool *pt,Wndo *w)
{
Errcode err;
Fcelpos lastpos;
SHORT dx,dy,omx,omy;
(void)pt;
(void)w;

	cel_cancel_undo();
	if(!thecel)
		return(Success);

	hide_mp();
	save_paste_undo();
	disable_toolcel_redraw();
	if(vs.cm_streamdraw)
		wait_penup();

	omx = icb.mx;
	omy = icb.my;
	cmu_unmarqi_cel();
	draw_flicel(thecel,DRAW_DELTA,OLD_CFIT);
	get_fcelpos(thecel,&lastpos);

	for(;;)
	{
		if(vs.cm_streamdraw && ISDOWN(MBPEN))
			check_input(MMOVE|KEYHIT|MBPEN|MBPUP|MBRIGHT);
		else
			wait_input(MMOVE|KEYHIT|MBPEN|MBPUP|MBRIGHT);

		dx = icb.mx - omx;
		dy = icb.my - omy;
		omx = icb.mx;
		omy = icb.my;

		if(JSTHIT(MBRIGHT|KEYHIT) && !ISDOWN(MBPEN)) /* user abort */
		{
			err = Err_abort;
			goto error;
		}
		if(JSTHIT(MBPEN) || (vs.cm_streamdraw && ISDOWN(MBPEN) && (dx || dy)))
		{
			/* have to bracket with poll atom because some 
			 * inks check abort !! */

			start_abort_atom();
			if(vl.ink->ot.id != opq_INKID || vs.make_mask || vs.use_mask)
			{
				if(JSTHIT(MBPEN))
					unsee_flicel(thecel);
				translate_flicel(thecel,dx,dy);
				draw_flicel(thecel,DRAW_RENDER,NEW_CFIT);
				dirties();
				save_fcel_undo(thecel);
				check_cel_ccycle();
				if(vs.paste_inc_cel)
					seek_fcel_frame(thecel, thecel->cd.cur_frame + 1);
				if(!vs.cm_streamdraw)
					draw_flicel(thecel,DRAW_FIRST,OLD_CFIT);
				get_fcelpos(thecel,&lastpos);
				end_abort_atom();
				continue;
			}
			end_abort_atom();

			/* single hit mode */

			if(JSTHIT(MBPEN))
			{
				if(JSTHIT(MMOVE)  
					|| ((thecel->flif.hdr.frame_count > 1)
						&& (vs.cm_streamdraw 
							 || (vs.cycle_draw && vs.render_one_color))))
				{
					do_delta_celcurs(dx,dy);
				}
				save_fcel_undo(thecel);
			}
			else /* in stream mode after first hit (mouse moved) */
			{
				translate_flicel(thecel,dx,dy);
				draw_flicel(thecel,DRAW_FIRST,NEW_CFIT);
				save_fcel_undo(thecel);
			}
			dirties();
			check_cel_ccycle();
			get_fcelpos(thecel,&lastpos);
			if(vs.paste_inc_cel)
			{
				seek_fcel_frame(thecel, thecel->cd.cur_frame + 1);
				if(!vs.cm_streamdraw)
					do_delta_celcurs(dx,dy);
			}
		}
		else if(JSTHIT(MMOVE) || (vs.cm_streamdraw && vs.paste_inc_cel))
			do_delta_celcurs(dx,dy);
	}
	err = Success;

error:
	unsee_flicel(thecel);
	save_undo();
	cmcb->undo_corrupted = FALSE;
	put_fcelpos(thecel,&lastpos);
	draw_flicel(thecel,DRAW_FIRST,NEW_CFIT);
	if(vs.cycle_draw && vs.render_one_color)
		do_color_redraw(NEW_CCOLOR);
	enable_toolcel_redraw();
	cmu_marqi_cel();
	show_mp();
	return(err);
}
