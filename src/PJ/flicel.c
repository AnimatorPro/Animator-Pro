/* flicel.c - Stuff to do the basic things you need to do to a cel (ie a
   rectangular image with a color map.	Flicels and Rcels (aka
   screens) are almost the same structure.	You can use all the cel
   routines on a screen as well.  There's some interactive cel positioning
   and pasting routines here too.  Also some of the other easy stuff on
   the Cel drop-down */

#include "jimk.h"
#include "auto.h"
#include "errcodes.h"
#include "flipath.h"
#include "floatgfx.h"
#include "flx.h"
#include "marqi.h"
#include "memory.h"
#include "pentools.h"
#include "poly.h"
#include "polyrub.h"
#include "rastlib.h"
#include "rastrans.h"
#include "render.h"
#include "zoom.h"

static void
changetocel(UBYTE *oldline, UBYTE *newline, Rcel *cel, SHORT y, Pixel tcolor);

#ifdef SLUFFED
LONG jwrite_celchunk(Jfile f,Celdata *cd)
/* write a celdata flichunk returns size of chunk if ok Errcode if not */
{
	cd->id.type = FP_CELDATA;
	cd->id.size = sizeof(Celdata);

	if(pj_write(f,cd,sizeof(Celdata) != sizeof(Celdata)))
		return(pj_ioerr());
	else
		return(sizeof(Celdata));
}
#endif /* SLUFFED */

static void init_celchunk(Celdata *cd)
{
	clear_mem(cd, sizeof(*cd));
	cd->id.type = FP_CELDATA;
	cd->id.size = sizeof(Celdata);
}
static void free_fcel_cfit(Flicel *fc)
{
	if(fc->flags & FCEL_OWNS_CFIT)
	{
		pj_freez(&fc->cfit);
		fc->flags &= ~FCEL_OWNS_CFIT;
	}
	fc->cfit = NULL;
}
static Errcode alloc_fcel_cfit(Flicel *fc)
{
	free_fcel_cfit(fc);
	if(NULL == (fc->cfit = pj_malloc(sizeof(Celcfit))))
		return(Err_no_memory);
	fc->flags |= FCEL_OWNS_CFIT;
	return(Success);
}
void free_fcel_raster(Flicel *fc)
{
	if(fc->flags & FCEL_OWNS_RAST)
	{
		pj_rcel_free(fc->rc);
		fc->rc = NULL;
	}
	fc->flags &= ~FCEL_OWNS_RAST;
}
Errcode alloc_fcel_raster(Flicel *fc)

/* will not allocate if already present */
{
Errcode err;

	if(fc->rc && (fc->flags & FCEL_OWNS_RAST))
		return(Success);
	fc->frame_loaded = !fc->cd.cur_frame; /* force reseek from start */
	err = valloc_ramcel(&fc->rc,fc->flif.hdr.width,fc->flif.hdr.height);
	if(err >= Success)
		fc->flags |= FCEL_OWNS_RAST;
	return(err);
}
void free_fcel(Flicel **pfc)
{
Flicel *fc;

	if((fc = *pfc) == NULL)
		return;
	free_fcel_raster(fc); /* only does if alloc'd and owned */
	pj_fli_close(&fc->flif); /* only does if open */
	free_flipath(&fc->cpath);
	free_fcel_cfit(fc);
	pj_freez(&fc->tpath);
	pj_freez(pfc);
}
void free_the_cel(void)
{
	free_fcel(&thecel);
}
static void noask_delete_the_cel(void)
{
	free_the_cel();
	pj_delete(cel_name);
	pj_delete(cel_fli_name);
	set_trd_maxmem();
}
void delete_the_cel(void)
{
	if (soft_yes_no_box("cel_del"))
		noask_delete_the_cel();
}
Errcode alloc_fcel(Flicel **pcel)

/* Try to find room on the heap for a flicel
 * sets *pcel to null if can't do and returns ecode 0 if ok */
{
Flicel *fc;
Errcode err;

	if(NULL == (*pcel = pj_zalloc(sizeof(Flicel))))
	{
		err = Err_no_memory;
		goto error;
	}
	fc = *pcel;
	init_celchunk(&fc->cd);
	init_xformspec(&fc->xf);
	fc->frame_loaded = -1;
	if((err = alloc_fcel_cfit(fc)) < Success)
		goto error;

	return(Success);
error:
	free_fcel(pcel);
	return(err);
}
void set_flicel_tcolor(Flicel *fc,Pixel tcolor)
{
	fc->cd.tcolor = tcolor;
}

/**** fcel positioning items *****/

void set_fcel_center(Flicel *fc, SHORT x, SHORT y)

/* set center and update */
{
	fc->cd.cent.x = x;
	fc->cd.cent.y = y;
	refresh_flicel_pos(fc);
}
void rotate_flicel(Flicel *fc, Short_xyz *drotate)
/* angles to be in FCEL_TWOPI units only about 3 turns will fit in a short */
{
	fc->cd.rotang.x += drotate->x;
	fc->cd.rotang.y += drotate->y;
	fc->cd.rotang.z += drotate->z;
}
void translate_flicel(Flicel *fc, SHORT dx,SHORT dy)
{
	fc->cd.cent.x += dx;
	fc->cd.cent.y += dy;
}
void clear_fcel_xform(Flicel *fc)
{
	fc->cd.rotang.x = fc->cd.rotang.y = fc->cd.rotang.z = 0;
	fc->cd.stretch.x = fc->cd.stretch.y = 0;
}
void fcelpos_to_box(Flicel *fc, Fcelpos *pos, Rectangle *box)
{
	pos->rotang.x = pos->rotang.y = pos->rotang.z = 0;
	pos->cent.x = box->x + (box->width>>1);
	pos->cent.y = box->y + (box->height>>1);
	pos->stretch.x = box->width - fc->flif.hdr.width;
	pos->stretch.y = box->height - fc->flif.hdr.height;
}
static void fcel_to_box(Flicel *fc, Rectangle *box)
{
	fcelpos_to_box(fc,((Fcelpos *)&((fc)->cd.CDAT_POS_START)),box);
}
void center_fcel_in_screen(Flicel *fcel,Rcel *screen)
{
	set_fcel_center(fcel, screen->width>>1, screen->height>>1);
}
void scale_fcel_to_screen(Flicel *fcel,Rcel *screen)
{
Rectangle box;

	box.width = screen->width;
	box.height = screen->height;
	box.x = box.y = 0;
	fcel_to_box(fcel,&box);
	center_fcel_in_screen(fcel,screen);
}
void save_fcel_undo(Flicel *fc)

/* saves area under cel to undo buffer plus an extra pixel beyond */
{
	maybe_ref_flicel_pos(fc);
	save_undo_rect(fc->xf.mmax.x - 1,fc->xf.mmax.y - 1,
				   fc->xf.mmax.width + 2,fc->xf.mmax.height + 2);
}
void unsee_flicel(Flicel *fc)
{
	zoom_undo_rect(fc->xf.ommax.x,fc->xf.ommax.y,
				   fc->xf.ommax.width,fc->xf.ommax.height);
}
void marqi_flicel(Flicel *fc, int dotmod, Pixel *save_buf)

/* if save buffer is non NULL it will put area behind marqi in buffer.
 * buffer must be big enough to contain all pixels of the marqi on the
 * screen and the four corner points. ((W*H)+4)*sizeof(Pixel) is enough */
{
Marqihdr mh;
Short_xy *pgpt;
(void)fc;

	vinit_marqihdr(&mh,1,1);
	mh.dmod = mh.smod = dotmod;
	if((mh.dotbuf = save_buf) == NULL)
		msome_vector(thecel->xf.bpoly,4,mh.pdot,&mh,0,sizeof(Short_xy));
	else
	{
		/* save four corners of cel rectangle */
		for(pgpt = &thecel->xf.bpoly[0];
			pgpt <= &thecel->xf.bpoly[3];
			++pgpt )
		{
			*save_buf++ = pj_get_dot(vb.pencel,pgpt->x, pgpt->y);
		}
		mh.dotbuf = (UBYTE *)save_buf;
		msome_vector(thecel->xf.bpoly,4,savedraw_marqidot,
					 &mh,0,sizeof(Short_xy));
	}
}
void undo_flicel_marqi(Flicel *fc,Pixel *save_buf)
{
Short_xy *pgpt;
Marqihdr mh;
(void)fc;

	vinit_marqihdr(&mh,1,1);
	if(NULL == (mh.dotbuf = save_buf))
		msome_vector(thecel->xf.bpoly,4,undo_marqidot,&mh,0,sizeof(Short_xy));
	else
	{
		mh.dotbuf = (UBYTE *)(save_buf + 4);
		msome_vector(thecel->xf.bpoly,4,restore_marqidot,&mh,
					 0,sizeof(Short_xy));

		/* restore four corners */
		for(pgpt = &thecel->xf.bpoly[0];
			pgpt <= &thecel->xf.bpoly[3];
			++pgpt )
		{
			pj_put_dot(vb.pencel,*save_buf++,pgpt->x, pgpt->y);
		}
	}
}
static Errcode draw_thecel_a_sec(void)
{
Errcode err;
Marqihdr mh;

	if((err = draw_flicel(thecel,DRAW_FIRST,NEW_CFIT)) < 0)
		goto error;

	vinit_marqihdr(&mh,1,1);
	mh.smod = 8;
	while (--mh.smod >= 0)
	{
		mh.dmod = mh.smod;
		marqi_vector(&mh,thecel->xf.bpoly,4,sizeof(Short_xy));
		wait_a_jiffy(4);
	}
	err = Success;
error:
	return(err);
}
Errcode show_thecel_a_sec(void)
{
Errcode err;

	if(thecel == NULL)
		return(Err_no_cel);

	save_undo();
	err = draw_thecel_a_sec();
	unsee_flicel(thecel);
	return(softerr(err,"cel_display"));
}
static Errcode clip_from_fli(char *tempname,char *fliname,
							 Flicel **pfcel,Rectangle *rect)

/* Create a flicel from a rectangular patch of the screen
 * note can only be called with &thecel since it builds temp file set
 * under tempname */
{
Errcode err;
Rcel *clip;

	if((err = clip_celrect(vb.pencel, rect, &clip)) < 0)
		goto error;

	if((err = make1_flicel(tempname, fliname, pfcel, clip)) < 0)
		goto error;

	return(0);
error:
	pj_rcel_free(clip);
	*pfcel = NULL;
	return(err);
}
typedef struct clipceldat {
	Rcel *rc;
	Flicel **pfcel;
	Rectangle start;
	Rectangle end;
	char *tempname;
	char *fliname;
	Flicel *old_cel;
} Clipceldat;

static Errcode
clip1_celframe(void *clipceldat, int ix, int it, int scale, Autoarg *aa)
{
Clipceldat *cd = clipceldat;
Errcode err;
int dx,dy;
Rectangle bounds;
Marqihdr mh;
Rcel clipcel;
Flicel *fc;
Rcel *rc;
Fli_frame *cbuf = NULL;
LONG oset;
(void)ix;
(void)it;

	dx = cd->end.x - cd->start.x;
	dy = cd->end.y - cd->start.y;
	bounds = cd->start;
	bounds.x += itmult(dx,scale);
	bounds.y += itmult(dy,scale);

	if(!aa->in_preview)
	{
		pj_rcel_make_virtual(&clipcel,vb.pencel,&bounds);

		if(aa->cur_frame == 0) /* doing first frame */
		{
			/* first time we have to delete old cel and make a new one */

			free_fcel(&cd->old_cel);
			if((err = clip_celrect(vb.pencel, &bounds, &rc)) < 0)
				goto error;

			if((err = create_celfli_start(cd->tempname,cd->fliname,
										  cd->pfcel,rc)) < 0)
			{
				pj_rcel_free(rc);
				goto error;
			}
			fc = *(cd->pfcel);
			fc->flif.hdr.speed = flix.hdr.speed; /* where it is coming from */
			fc->flif.hdr.id = flix.hdr.id; /* you didn't really create this,
											* did you ?? */

			set_fcel_center(fc,clipcel.x + (clipcel.width>>1),
							   clipcel.y + (clipcel.height>>1));
		}
		else
			fc = *(cd->pfcel);

		if((err = pj_fli_cel_alloc_cbuf(&cbuf,&clipcel)) < 0)
			goto error;

		if(aa->cur_frame == 0) /* write first frame */
		{
			err = pj_fli_add_frame1(cd->fliname, &fc->flif, cbuf, &clipcel);
		}
		else /* write next frame */
		{
			err = pj_fli_add_next(cd->fliname, &fc->flif,cbuf,fc->rc,&clipcel);
		}

		if(err < Success)
			goto error;

		if(aa->cur_frame == (aa->frames_in_seq - 1)) /* last frame, ringit */
		{
			if(aa->cur_frame != 0)
			{
				/* gotta bo back and get first frame of cel fli */

				if((err = pj_i_read_uncomp1(cd->fliname,&fc->flif,
											  fc->rc, cbuf, 0)) < 0)
				{
					goto error;
				}

				/* go back to end of file and write ring frame and finish */

				if((oset = pj_seek(fc->flif.fd,0,JSEEK_END)) < 0)
				{
					err = oset;
					goto error;
				}
			}
			if((err = pj_fli_add_ring(cd->fliname, &fc->flif, cbuf,
									   &clipcel, fc->rc)) < Success)
			{
				goto error;
			}
			pj_fli_close(&fc->flif);
			/* leave first frame in cel */
		}
		else /* there is a next frame so copy current clip to cel */
		{
			pj_cmap_copy(clipcel.cmap, fc->rc->cmap);
			pj_blitrect(&clipcel,0,0,fc->rc,0,0,clipcel.width,clipcel.height);
		}
	}

	vinit_marqihdr(&mh,1,1);
	marqi_rect(&mh,&bounds);

	err = Success;
error:
	pj_gentle_free(cbuf);
	return(err);
}
static Errcode multi_fli_clip(char *tempname, char *fliname,
							  Flicel **pfcel,Rectangle *r, Boolean render_only)

/* note *pfcel must be NULL unless it contains the old cel to be freed and
 * replaced by the new one */
{
Errcode err;
Clipceldat cd;
Flicel *fc;
Autoarg aa;

	flx_clear_olays(); /* undraw cels cursors etc */

	clear_struct(&cd);
	cd.old_cel = *pfcel;
	cd.pfcel = pfcel;
	cd.end = *r;
	cd.start = *r;
	cd.fliname = fliname;
	cd.tempname = tempname;
	*pfcel = NULL;

	push_cel();  /* disable push/pop or get rid of it here */

	if(!render_only)
	{
		if((err = rect_in_place(&cd.start)) >= Success)
			err = clip_move_rect(&cd.end);
		if(err < 0 && err != Err_abort)
			goto error;
	}


	clear_struct(&aa);
	aa.avec = clip1_celframe;
	aa.avecdat = &cd;
	aa.flags = AUTO_READONLY|AUTO_UNZOOM;

	if(render_only)
		err = noask_do_auto_time_mode(&aa);
	else
		err = do_auto(&aa);

	if(err < Success)
		goto error;

	/* write a temp info file for this cel's new fli */

	fc = *cd.pfcel;
	set_flicel_tcolor(fc,vs.inks[0]); /* set to current tcolor */

	if((err = save_fcel_temp(fc)) < Success)
		goto error;

	goto ok_out;
error:
	free_fcel(pfcel);
	if(cd.old_cel) /* it freed freed before new files created */
	{
		*pfcel = cd.old_cel;
	}
	else
	{
		pj_delete(tempname);
		pj_delete(fliname);
	}

ok_out:
	pop_cel(); /* re-enable push/popping or restore old "thecel" */
	flx_draw_olays();
	return(err);
}
/*******************************************************/

typedef struct clipper_rast {
	Rcel rc;
	Pixel tcolor;
	Raster *root;
} Crast;

static void clpdot(Crast *cr,Pixel color,Coor x,Coor y)
{
	if(color == cr->tcolor)
		return;
	pj__put_dot(cr->root,color,x,y);
}
static void clsethline(Crast *cr,Pixel color,Coor x,Coor y,Ucoor width)
{
	if(color == cr->tcolor)
		return;
	pj__put_dot(cr->root,color,x,y);
	pj__put_dot(cr->root,color,x + width - 1,y);
}
static void clphseg(Crast *cr,Pixel *pbuf,Coor x,Coor y,Coor width)
{
	while(width-- > 0)
	{
		if(*pbuf++ != cr->tcolor)
		{
			pj__put_dot(cr->root,pbuf[-1],x,y);
			pbuf += width - 1;
			++width;
			break;
		}
		++x;
	}
	while(width-- > 0)
	{
		if(*pbuf-- != cr->tcolor)
		{
			pj__put_dot(cr->root,pbuf[1],x + width,y);
			break;
		}
	}
}

static Pixel nogetd(Raster *r, Coor x, Coor y)
{
	(void)r;
	(void)x;
	(void)y;

	return(0);
}
static Errcode find_segment_clip(int from, int to, Rectangle *bounds)

/* finds "clip" rectangle starting with from and going to "to" alters
 * frame_ix so you must save it if you want it */
{
Errcode err;
Crast *cliprast;
struct rastlib *lib;

	/* save current state */

	if((err = scrub_cur_frame()) < Success)
		return(err);

	/* make up a "clipper rast" */

	if(NULL == (cliprast = pj_zalloc(sizeof(Crast) + sizeof(Rastlib))))
		return(Err_no_memory);

	cliprast->rc = *vb.pencel;
	cliprast->rc.lib = lib = (struct rastlib *)(cliprast+1);
	lib->put_dot = (rl_type_put_dot)clpdot;
	lib->get_dot = nogetd;
	lib->put_hseg =(rl_type_put_hseg) clphseg;
	lib->set_hline = (rl_type_set_hline)clsethline;
	pj_set_grc_calls(lib);
	cliprast->rc.type = RT_CLIPBOX;
	cliprast->root = (Raster *)vb.pencel;
	cliprast->tcolor = vs.inks[0];


	if(from > to)
	{
	int swapr;

		swapr = to;
		to = from;
		from = swapr;
	}

	save_undo();
	pj_set_rast(vb.pencel,vs.inks[0]);
	fli_tseek(undof, vs.frame_ix, from);
	vs.frame_ix = from;

	pj_blitrect(undof,0,0,cliprast,0,0,vb.pencel->width,vb.pencel->height);

	start_abort_atom();
	while(from < to)
	{
		if((err = poll_abort()) < Success)
		{
			if(soft_yes_no_box("clip_abort"))
				break;
		}
		if ((err = flx_ringseek(&cliprast->rc, from, from+1)) < Success)
			break;
		++from;
	}
	if((err = errend_abort_atom(err)) < Success)
		goto error;

	if((err = find_clip(vb.pencel, bounds, vs.inks[0])) < Success)
		goto error;

	if(!bounds->height)
		err = Err_clipped;

error:
	zoom_unundo();
	pj_free(cliprast);
	return(err);
}
static Errcode cel_cant_clip(Errcode err)
{
	return(softerr(err,vs.multi?"cel_mfclip":"cel_clip"));
}
Errcode clip_cel(void)

/* Put the bits of screen not the key color into THE cel
 * returns ecode if can't do */
{
Rectangle bounds;
Errcode err;
extern Errcode pj_errdo_success();
int oframe_ix;

	flx_clear_olays(); /* undraw cels cursors etc */
	if(vs.multi && flix.hdr.frame_count > 1)
	{
		oframe_ix = vs.frame_ix;
		if ((err = go_autodraw((autoarg_func)pj_errdo_success, NULL,
							  AUTO_READONLY|AUTO_PREVIEW_ONLY|AUTO_UNZOOM)) < 0)
		{
			goto error;
		}

		switch(vs.time_mode)
		{
			case DOAUTO_ALL:
				err = find_segment_clip(0,flix.hdr.frame_count - 1,&bounds);
				break;
			case DOAUTO_SEGMENT:
				clip_tseg();
				err = find_segment_clip(vs.start_seg,vs.stop_seg,&bounds);
				break;
			case DOAUTO_FRAME:
				goto single_frame;
		}

		if(err >= Success)
		{
			err = multi_fli_clip(cel_name,cel_fli_name,&thecel,&bounds,TRUE);
			if(oframe_ix == vs.frame_ix)
				goto error;
		}
		fli_abs_tseek(undof, oframe_ix);
		zoom_unundo();
		vs.frame_ix = oframe_ix;
		goto error;
	}


single_frame:

	if((err = find_clip(vb.pencel, &bounds, vs.inks[0])) < 0)
		goto error;

	if(bounds.height)
	{
		free_the_cel();
		if((err = clip_from_fli(cel_name,cel_fli_name,&thecel,&bounds)) < 0)
			goto error;
		show_cel_a_sec(thecel->rc);
	}

error:
	flx_draw_olays();
	return(cel_cant_clip(err));
}

static Errcode cel_from_rect(Rectangle *rect, Boolean render_only)
{
Errcode err;

	if(vs.multi)
	{
		err = multi_fli_clip(cel_name,cel_fli_name,&thecel,rect,render_only);
	}
	else
	{
		free_the_cel();
		err = clip_from_fli(cel_name,cel_fli_name,&thecel,rect);
	}
	return(cel_cant_clip(err));
}

Errcode cut_out_cel(void)
/* Have user define a box and then grab a cel from that box.  Aka
   "get cel" */
{
Rectangle rect;
Errcode err;

	if((err = cut_out_rect(&rect)) >= Success)
		err = cel_from_rect(&rect,FALSE);

	return(err);
}

void qget_changes(void)
{
Errcode err;
Rcel *chgcel = NULL;
Rectangle bounds;
UBYTE *newline;
UBYTE *oldline;
SHORT cely;
Rcel_save opic;

	if ((err = report_temp_save_rcel(&opic, vb.pencel)) < Success)
		goto error;

	fli_abs_tseek(undof,vs.frame_ix); /* put the unchanged screen into uf */

	/* xor with changed screen and find dimensions of changed result */
	pj_xor_rast(undof, vb.pencel);

	find_clip(vb.pencel,&bounds,(Pixel)0);
	report_temp_restore_rcel(&opic, vb.pencel);

	free_the_cel();
	if(!bounds.height)
		goto done;

	if((err = clip_celrect(vb.pencel, &bounds, &chgcel)) < Success)
		goto error;

	/* we assume that the undof and the pencel are the same size */

	if((newline = pj_malloc(bounds.width * 2)) == NULL)
	{
		err = Err_no_memory;
		goto error;
	}

	oldline = newline + bounds.width;

	for(cely = 0;cely < bounds.height; ++cely)
	{
		pj__get_hseg(vb.pencel,newline,bounds.x,bounds.y,bounds.width);
		pj__get_hseg(undof,oldline,bounds.x,bounds.y,bounds.width);
		changetocel(oldline,newline,chgcel,cely,vs.inks[0]);
		++bounds.y;
	}
	pj_free(newline);

	if((err = make1_flicel(cel_name, cel_fli_name, &thecel, chgcel)) < Success)
		goto error;
	chgcel = NULL;

	show_thecel_a_sec();
	goto done;

error:
	pj_rcel_free(chgcel);
	free_the_cel();
	softerr(err,"eclip_changes");
done:
	return;
}


typedef struct lasso_dat {
	Pixel *lbuf;
	Rcel *src;
	Rcel *dst;
	SHORT xoff;
	SHORT yoff;
} Lasso_dat;

static Errcode lasso_line(SHORT y, SHORT xmin, SHORT xmax, void *data)
{
Lasso_dat *ld = (Lasso_dat *)data;
Coor width;
Pixel *lbuf;

	lbuf = ld->lbuf;
	pj_put_dot(vb.pencel,swhite,xmin,y);
	pj_put_dot(vb.pencel,swhite,xmax,y);
	width = xmax - xmin + 1;
	xmin += ld->xoff;
	xmax += ld->xoff;
	y += ld->yoff;
	pj_get_hseg(ld->src,lbuf,xmin,y,width);
	pj_put_hseg(ld->dst,lbuf,xmin,y,width);
	return(Success);
}

Errcode lasso_cel(void)
/* Have user define a ploygon shape then grab a cel from within shape */
{
Errcode err;
Poly shape;
Rcel *undosave;
Cliprect bounds;
Rectangle celsize;
Rcel *rc = NULL;
Rcel clipcel;
Lasso_dat ldat;
Boolean oclear;
LONG lbufsize;

	flx_clear_olays(); /* undraw cels cursors etc */
	clear_struct(&ldat);
	undosave = clone_rcel(undof);
	save_undo();
	wait_wndo_input(ANY_CLICK);
	if(!tti_input())
	{
		pj_rcel_free(undosave);
		err = Err_abort;
		goto out;
	}
	free_the_cel();
	push_cel();  /* disable push/pop here */

	clear_struct(&shape);
	if((err = get_rub_shape(&shape,swhite,sblack)) < 0)
		goto error;

	poly_bounds(&shape, &bounds);

	lbufsize = bounds.MaxX - bounds.x;

	if(bounds.x < 0)
		bounds.x = 0;
	if(bounds.y < 0)
		bounds.y = 0;
	if(bounds.MaxX > undof->width)
		bounds.MaxX = undof->width;
	if(bounds.MaxY > undof->height)
		bounds.MaxY = undof->height;

	crect_torect(&bounds,&celsize);

	if(celsize.width <= 0 || celsize.height <= 0)
	{
		err = Err_abort;
		goto error;
	}

	if((err = valloc_ramcel(&rc,celsize.width,celsize.height)) < 0)
		goto error;
	pj_cmap_copy(vb.pencel->cmap,rc->cmap); /* copy in cmap */
	pj_set_rast(rc,vs.inks[0]); /* fill with tcolor */

	pj_rcel_make_virtual(&clipcel,undof,&celsize);
	rc->x = celsize.x;
	rc->y = celsize.y;

	/* setup lasso_dat and do a filled poly */

	/* get maximum we might need */

	if((ldat.lbuf = pj_malloc(lbufsize * sizeof(Pixel))) == NULL)
	{
		err = Err_no_memory;
		goto error;
	}
	ldat.src = &clipcel;
	ldat.dst = rc;
	ldat.xoff = -celsize.x;
	ldat.yoff = -celsize.y;
	if((err = fill_poly_inside(&shape, lasso_line, &ldat)) < 0)
		goto error;

	if((err = make1_flicel(cel_name, cel_fli_name, &thecel, rc)) < 0)
		goto error;

	rc = NULL; /* now in thecel */
	oclear = vs.zero_clear;
	vs.zero_clear = 0;
	err = draw_thecel_a_sec(); /* if we can't do this forget it */
	vs.zero_clear = oclear;

error:
	pj_gentle_free(ldat.lbuf);
	free_polypoints(&shape);
	pj_rcel_free(rc);
	zoom_unundo();
	if(err < Success && undosave)
		pj_rcel_copy(undosave,undof);
	pj_rcel_free(undosave);
	err = cel_cant_clip(err);
	pop_cel();
	if(err >= Success) /* save and realloc cel to defrag */
	{
		push_cel();
		pop_cel();
	}
out:
	flx_draw_olays();
	return(err);
}


/************************************************************/
Boolean fcel_stretchsize(Flicel *cel, Srect *cr)

/* gets cel dimensions and origin on screen if cel is stretched.  This
 * does not apply rotation to the dimensions.  If width or height is
 * inverted (negative) this means the cel is reversed on this axis.
 * Note that an Srect is signed for width and height */
{
Rcel *ccel;
Boolean ret;

	ccel = cel->rc;
	cr->width = ccel->width;
	cr->height = ccel->height;
	cr->x = ccel->x = cel->cd.cent.x - (cr->width>>1);
	cr->y = ccel->y = cel->cd.cent.y - (cr->height>>1);

	if(cel->cd.stretch.x)
	{
		cr->width += cel->cd.stretch.x;
		cr->x = cel->cd.cent.x-(cr->width>>1);
		ret = TRUE;
	}
	else
		ret = FALSE;

	if(cel->cd.stretch.y)
	{
		cr->height += cel->cd.stretch.y;
		cr->y = cel->cd.cent.y-(cr->height>>1);
		ret = TRUE;
	}
	return(ret);
}

Boolean maybe_ref_flicel_pos(Flicel *cel)

/* does all calculations to refresh flicel position dependent vars when
 * data in the flicel->cd has been altered builds polygon and sets
 * position dependent variables returns TRUE if cel is transformed
 * FALSE if not */
{
Boolean ret;
ULONG cksum;
Srect cr;

	/* clip rotation angle */

	if((cel->cd.rotang.x = cel->cd.rotang.x % FCEL_TWOPI) < 0)
		cel->cd.rotang.x += FCEL_TWOPI;
	if((cel->cd.rotang.y = cel->cd.rotang.y % FCEL_TWOPI) < 0)
		cel->cd.rotang.y += FCEL_TWOPI;
	if((cel->cd.rotang.z = cel->cd.rotang.z % FCEL_TWOPI) < 0)
		cel->cd.rotang.z += FCEL_TWOPI;

	if(cel->pos_cksum == (cksum = mem_crcsum(&cel->cd.CDAT_POS_START,
											sizeof(Fcelpos))))
	{
		return(cel->flags & FCEL_XFORMED);
	}
	cel->pos_cksum = cksum;

	ret = fcel_stretchsize(cel, &cr);
	if(cr.width < 0)
		--cr.x;
	if(cr.height < 0)
		--cr.y;

	if(cel->cd.rotang.z)
	{
		sq_poly(cr.width, cr.height, cr.x, cr.y, cel->xf.bpoly);

		frotate_points2d(itheta_tofloat(cel->cd.rotang.z,FCEL_TWOPI),
						 &cel->cd.cent,
						 (Short_xy *)(cel->xf.bpoly),
						 (Short_xy *)(cel->xf.bpoly),
						  4 );
		++ret;
	}
	else
		sq_poly(cr.width, cr.height, cr.x, cr.y, cel->xf.bpoly);

	if(ret)
	{
		load_poly_minmax(&cel->xf);
		cel->flags |= FCEL_XFORMED;
	}
	else
	{
		load_rect_minmax((Rectangle *)&(cel->rc->RECTSTART),&cel->xf);
		cel->flags &= ~FCEL_XFORMED;
	}
	return(ret);
}
Boolean refresh_flicel_pos(Flicel *cel)
{
	cel->pos_cksum = -1;
	return(maybe_ref_flicel_pos(cel));
}

static void zundraw_line(Coor x, Coor y, Ucoor width, Pixel *lbuf)
{
	pj_get_hseg(undof,lbuf,x,y,width);
	pj_put_hseg(vb.pencel,lbuf,x,y,width);
	if(vs.zoom_open)
		zoom_put_hseg((Raster *)vb.pencel, lbuf, x, y, width);
}

typedef struct plinedat {
	Procline pline;
	Tcolxldat tcxl;
	Pixel *lbuf;
	Rcel *blitsrc;
} Plinedat;

static Errcode fcel_putline(void *plinedat, Pixel *line,
							Coor x, Coor y, Ucoor width)
{
	Plinedat *pd = plinedat;

	if(pd->pline == NULL)
	{
		if(pd->tcxl.xlat != NULL)
			pj_xlate(pd->tcxl.xlat,line,width);
	}
	else
	{
		pj_get_hseg(pd->blitsrc,pd->lbuf,x,y,width);
		(*pd->pline)(line,pd->lbuf,width,&pd->tcxl);
		line = pd->lbuf;
	}
	pj_put_hseg(vb.pencel,line,x,y,width);
	if(vs.zoom_open)
		zoom_put_hseg((Raster *)vb.pencel, line, x, y, width);
	return(0);
}

Errcode draw_flicel(Flicel *fc, int drawmode, int cfitmode)
{
Errcode err;
Celblit blit;
Celcfit *cfit;
Plinedat pld;
Rcel *rc;
extern char under_flag;

	rc = fc->rc;
	maybe_ref_flicel_pos(fc);
	pld.tcxl.xlat = NULL;

	if(NULL != (cfit = fc->cfit))
	{
		switch(cfitmode)
		{
			case FORCE_CFIT:
				cfit->ccolor = -1;
				cfit->src_cksum = 0x7FFFFFFF;
				cfit->dst_cksum = 0;
			case NEW_CFIT:
				make_render_cfit(rc->cmap,cfit,fc->cd.tcolor);
			case OLD_CFIT:
				if(!(cfit->flags & CCFIT_NULL))
					pld.tcxl.xlat = cfit->ctable;
			case NO_CFIT:
				break;
		}
	}

	under_flag = vs.render_under;
	if(vs.render_under)
		pld.tcxl.tcolor = vs.inks[0];
	else
		pld.tcxl.tcolor = fc->cd.tcolor;

	if( fc->flags & FCEL_XFORMED
		|| ((drawmode != DRAW_FIRST)
			 && ( fc->xf.ommax.width != fc->xf.mmax.width
				   || fc->xf.ommax.height != fc->xf.mmax.height)))
	{

		switch(drawmode)
		{
			case DRAW_FIRST:
				pld.blitsrc = vb.pencel;
				goto src_set;
			case DRAW_DELTA:
				pld.blitsrc = undof;
			src_set:
				pld.pline = get_celprocline(pld.tcxl.xlat != NULL);
				pld.lbuf = pj_malloc(Max(fc->xf.mmax.width,fc->xf.ommax.width)
									* sizeof(Pixel));
				if(pld.lbuf == NULL)
				{
					err = Err_no_memory;
					goto done;
				}
				err = raster_transform(rc,vb.pencel,&fc->xf,
									   fcel_putline,
									   &pld,drawmode == DRAW_DELTA,
									   zundraw_line, zoom_undo_rect,pld.lbuf);
				pj_free(pld.lbuf);
				goto done;
			case DRAW_RENDER:
				err = render_transform(rc,&fc->xf,&pld.tcxl);
				goto done;
			default:
				err = Err_bad_input;
				goto done;
		}
	}

	/* not transformed */

	switch(drawmode)
	{
		case DRAW_DELTA:
			do_leftbehind(fc->xf.ommax.x,fc->xf.ommax.y,
						  fc->xf.mmax.x,fc->xf.mmax.y,
						  fc->xf.ommax.width,fc->xf.ommax.height,
						  (do_leftbehind_func)undo_rect);
			blit = get_celmove(pld.tcxl.xlat != NULL);
			goto blitit;
		case DRAW_FIRST:
			blit = get_celblit(pld.tcxl.xlat != NULL);
		blitit:
			(*blit)(rc, 0, 0, vb.pencel, rc->x, rc->y,
					rc->width, rc->height, &pld.tcxl, undof);
			if(vs.zoom_open)
			{
				if(drawmode == DRAW_DELTA)
				{
					do_leftbehind(fc->xf.ommax.x,fc->xf.ommax.y,
								  fc->xf.mmax.x,fc->xf.mmax.y,
								  fc->xf.mmax.width,fc->xf.mmax.height,
								  (do_leftbehind_func)rect_zoom_it);
				}
				zoom_cel(rc);
			}
			err = Success;
			break;
		case DRAW_RENDER:
			err = rblit_cel(rc, &pld.tcxl);
			break;
	}

done:
	fc->xf.ommax = fc->xf.mmax;
	under_flag = 0;
#ifdef TESTING
	errline(err, "err draw flicel!");
#endif
	return(err);
}

#ifdef WITH_POCO
Errcode render_thecel()
{
	if (thecel == NULL)
		return(Err_abort);
	return(draw_flicel(thecel,DRAW_RENDER,NEW_CFIT));
}
#endif /* WITH_POCO */

#ifdef SLUFFED
void qfind_the_cel()

/* center the cel in visible area of screen */
{
Rectangle port;
Short_xy poly[5];
Short_xy *point;
Rcel *cel;


	if(thecel)
	{
		copy_mem(thecel->xf.bpoly,poly,sizeof(thecel->xf.bpoly));
		poly[4] = thecel->cd.cent;
		cel = thecel->rc;
		hide_mp();
		for(point = &poly[0];point <= &poly[4];++point)
		{
			if(wndo_dot_visible(PENWNDO,point->x,point->y))
				goto showit;
		}

		get_penwndo_port(&port);
		set_fcel_center(thecel,port.x+(port.width>>1),
								 port.y+(port.height>>1));
	showit:
		show_thecel_a_sec();
		show_mp();
	}
}
#endif /* SLUFFED */

static void changetocel(UBYTE *oldline,
						UBYTE *newline,
						Rcel *cel, SHORT y,
						Pixel tcolor )

/* removes all unchanged pixels in newline (compared with oldline) and replaces
 * them with tcolor */
{
UBYTE c;
UBYTE *d;
UBYTE *endpix;

	if(cel->type != RT_BYTEMAP)
		softerr(Err_unimpl, "cel_change");

	d = ((Bytemap *)cel)->bm.bp[0] + (((Bytemap *)cel)->bm.bpr * y);
	endpix = d + ((Bytemap *)cel)->bm.bpr;

	while(d < endpix)
	{
		c = *newline++;
		if(c == *oldline++)
			*d++ = (UBYTE)tcolor;
		else
			*d++ = c;
	}
}
