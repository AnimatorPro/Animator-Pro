/* auto.c - This file contains the basic methods for doing an operation

   over multiple frames.  The key function is doauto(rvec).  Rvec is
   a function which is called with three parameters:
   		the first is which of the multiple frames it dealing with
		the second is how many frames total there are to deal with
		the last is a scaled percentage indicating how far along
		    in the sequence of frames it is.  (0 for first frame,
			SCALE_ONE (or 1<<14 literally) for the last frame.)
   Rvec is expected to do some transformation on the screen *vb.pencel.
   Rvec also has access to the screen undof (the undo buffer) which will
   start out as a copy of render form.

   Basically rvec just draws on the screen as if it were doing it to
   a single frame,  using the scale parameter to figure out where
   to draw if it wants to move something.

   Also included in this file are some of the simpler functions that
   call do_auto and their associated rvec functions.  By convention
   a rvec function name ends with '1' - eg expand1.

   dopreview(rvec) is much like doauto.  (At least rvec is the same.)
   Instead of actually changing the flic it just does rvec and throws
   away the result.

   all rvec functions return 0 if all is well (Ecode < 0) if not */

#include "jimk.h"
#include "auto.h"
#include "bhash.h"
#include "errcodes.h"
#include "flx.h"
#include "inks.h"

static Errcode dall(Autoarg *aa);

Boolean auto_abort_verify(Autoarg *aa)
/* verify they'd like to call rendering off... */
{
	see_cmap(); /* some conditions may leave cmap out of sync */
	return(soft_yes_no_box("!%u%u", "auto_abort",
					   		aa->cur_frame, aa->frames_in_seq));
}
static void rpmuzauto(EFUNC what,void *dat)

/* Free up lots of memory by swapping out the world to disk.  Then
   doauto. */
{
	go_autodraw(what,dat,(AUTO_UNZOOM|AUTO_HIDEMP|AUTO_PUSHMOST));
}

void pmhmpauto(EFUNC what,void *dat)
{
	go_autodraw(what,dat,(AUTO_UNZOOM|AUTO_HIDEMP|AUTO_PUSHMOST));
	/*
	flx_clear_olays();
	push_most();
	hmpauto(what,dat);
	pop_most();
	flx_draw_olays();
	*/
}


void hmpauto(EFUNC what,void *dat)
/* Hide menus before do auto */
{
	go_autodraw(what,dat,(AUTO_HIDEMP|AUTO_UNZOOM));
}

Errcode uzauto(EFUNC what,void *dat)
/* close zoom window before doauto */
{
	return(go_autodraw(what,dat,AUTO_UNZOOM));
}


static Errcode setpic1(UBYTE *color)
/* Clear screen rvec */
{
	pj_set_rast(vb.pencel,(Pixel)*color);
	return(Success);
}
static void clear_one_frame()
{
	save_undo();
	setpic1(&vs.inks[0]);
	zoom_it();
	dirties();
	return;
}
void clear_pic(void)
{

	if(!vs.multi)
	{
		clear_one_frame();
		return;
	}

	hide_mp();
	unzoom();

	/* only for preview, render returns Success */ 
	if((go_autodraw(setpic1,&vs.inks[0],AUTO_PREVIEW_ONLY)) < Success)
		goto out;

	switch(vs.time_mode)
	{
		case DOAUTO_FRAME:
			clear_one_frame();
			break;
		case DOAUTO_SEGMENT:
		{
		Autoarg aa;

			clear_struct(&aa);
			aa.avec = setpic1;
			aa.avecdat = &vs.inks[0];
			aa.flags = AUTO_UNZOOM|AUTO_HIDEMP|AUTO_PUSHMOST;
			noask_do_auto_time_mode(&aa);
			break;
		}
		case DOAUTO_ALL:
			empty_cleared_flx(vs.inks[0]); /* in tempflx.c */
			break;
	}

out:
	rezoom();
	show_mp();
}

static Errcode blue_num1(void *dat, SHORT ix)
/* Blue numbers rvec */
{
char buf[16];
int ypos;

	sprintf(buf, "%4d", ix+1);
	ypos = vb.pencel->width - fstring_width(vb.screen->mufont,"a9999");
	gftext(vb.pencel,vb.screen->mufont,buf,ypos,1,vs.inks[1],TM_MASK1);
	return(Success);
}

void auto_blue_nums(void)
{
Autoarg aa;

	clear_struct(&aa);
	aa.avec = blue_num1;
	aa.flags = AUTO_UNZOOM|AUTO_HIDEMP|AUTO_PUSHMOST;
	noask_do_auto(&aa, DOAUTO_ALL);
}


/* Trails stuff */
typedef struct traildat {
	Rcel *tscreen;
	SHORT ttype;
	int tcount;
} Traildat;

static Errcode trail1(Traildat *td)

/* returns Ecode */
{
Errcode err;
int ink;
int percent;

	err = Success;
	if(td->tcount)
	{
	percent = vl.ink->strength;
	if (percent == 0)
		percent = 50;
		switch (td->ttype)
		{
			case 0:
				err = transpblit(td->tscreen, 0, 0, percent);
				pj_rcel_copy(vb.pencel, td->tscreen);
				break;
			case 1:
			case 2:
				ink = (td->ttype == 1 ? 0 : vs.inks[0]);

				pj_tblitrect(vb.pencel,0,0,td->tscreen,0,0,
						  vb.pencel->width,vb.pencel->height,ink);

				pj_rcel_copy(td->tscreen, vb.pencel);
				break;
		}
	}
	++td->tcount;
	return(err);
}

int auto_trails(void)
{
Autoarg aa;
Traildat td;

	clear_struct(&aa);
	aa.avec = trail1;
	aa.avecdat = &td;

	unzoom();
	push_most();

	td.tcount = 0;
	if(alloc_pencel(&td.tscreen) < 0)
		return(0);
	if ((td.ttype = soft_qchoice(NULL, "trails")) >= 0)
	{
		fli_abs_tseek(td.tscreen,0);
		noask_do_auto(&aa,DOAUTO_ALL);
	}
	pj_rcel_free(td.tscreen);
	pop_most();
	rezoom();

	return(1);
}

/* Greys only stuff */
static Errcode grey1(void)
{
unsigned char ttable[COLORS];
int i;
Rgb3 *rgb;

	rgb = vb.pencel->cmap->ctab;
	for (i=0; i<COLORS; i++)
	{
		if ((ttable[i] = (rgb->r + rgb->g + rgb->b + 1)/3) > COLORS-1)
			ttable[i] = COLORS-1;
		++rgb;
	}
	rgb = vb.pencel->cmap->ctab;
	for (i=0; i<COLORS-1; i++)
	{
		rgb->r = rgb->g = rgb->b = i;
		++rgb;
	}
	rgb->r = RGB_MAX-1;
	rgb->g = rgb->b = 0;
	xlat_rast(vb.pencel,ttable,1);
	return(Success);
}

void greys_only(void)
{
rpmuzauto(grey1,NULL);
}

/* Lace and Engrave stuff */
#define BIGC ((RGB_MAX-1)*3)

static engrave;	/* is it engrave or dither? */

static Errcode dither1(void)
{
SHORT *lb[2];
int bix;
int x,y;
Rgb3 *color;
Rgb3 *ctab;
int c;
int cerr;
SHORT *tp, *np;
LONG sz;

	sz = (2*sizeof(SHORT))*(vb.pencel->width+2);

	if ((lb[0] = pj_malloc(sz)) == NULL)
		return(Err_no_memory);
	clear_mem(lb[0], sz);
	lb[1] = lb[0]+vb.pencel->width+2;
	bix = 0;
	ctab = vb.pencel->cmap->ctab;

	for (y=0; y<vb.pencel->height; y++)
	{
		tp = lb[bix]+1;
		bix = 1-bix;
		np = lb[bix]+1;
		np[0] = 0;
		np[1] = 0;
		for (x=0; x<vb.pencel->width; x++)
		{
			c = *tp++;
			color = ctab + pj_get_dot(vb.pencel,x,y);
			c += color->r;
			c += color->g;
			c += color->b;
			++color;
			if (c>=BIGC/2)
			{
				pj_put_dot(vb.pencel,1,x,y);
				cerr = c - BIGC;
			}
			else
			{
				pj_put_dot(vb.pencel,0,x,y);
				cerr = c;
			}
			if (engrave)	/* originally a bug but I liked it */
			{
				c = (cerr+2)/4;	/* rounded 1/4ths */
				*tp += c;
				*(np-1) = c;
				*np++ = c;
				*np = cerr - c - c - c;
			}
			else
			{
				c = (3*cerr+4)/8;	/* rounded 3/8ths */
				*tp += c;
				*np++ += c;
				*np = cerr - c - c;
#ifdef FS
				c = (cerr+2)/4;	/* rounded 1/4ths */
				*tp += c;
				*(np-1) += c;
				*np++ += c;
				*np = cerr - c - c - c;
#endif /* FS */
			}
		}
	}
	set_color_rgb(&pure_black,0,vb.pencel->cmap);
	set_color_rgb(&pure_white,1,vb.pencel->cmap);
	pj_free(lb[0]);
	return(Success);
}

static void auto_de(void)
{
	rpmuzauto(dither1,NULL);
}

void auto_engrave(void)
{
	engrave = 1;
	auto_de();
}

void auto_dither(void)
{
	engrave = 0;
	auto_de();
}

/* Put alt stuff */
static int put_alt1(void)
{
	pj_rcel_copy(vl.alt_cel, vb.pencel);
	return(0);
}

void auto_put(void)
{
	if (vl.alt_cel)
		go_autodraw(put_alt1,NULL,(AUTO_UNZOOM|AUTO_PUSHCEL));
}

/* Apply ink stuff */
static Errcode render_set1(void)
{
	return(render_box(0,0,vb.pencel->width,vb.pencel->height));
}
void auto_set(void)
{
USHORT flags;

	flags = AUTO_UNZOOM;
	if(vl.ink->needs & INK_NEEDS_CEL)
		flags |= AUTO_USESCEL;
	go_autodraw(render_set1,NULL,flags);
}

static Errcode crop1(Cliprect *crop)

/* render all but crop Cliprect */
{
SHORT omode, ocolor;

	omode = vs.ink_id;
	ocolor = vs.ccolor;
	id_set_curink(opq_INKID);
	vs.ccolor = vs.inks[0];
	render_box(0,0,vb.pencel->width-1,crop->y-1);
	render_box(0,crop->y,crop->x-1,crop->MaxY-1);
	render_box(crop->MaxX,crop->y,vb.pencel->width-1,crop->MaxY-1);
	render_box(0,crop->MaxY,vb.pencel->width-1,vb.pencel->height-1);
	id_set_curink(omode);
	vs.ccolor = ocolor;
	return(Success);
}

void crop_video(void)
{
Cliprect crop;

	if(cut_out_clip(&crop) < 0)
		return;
	save_undo();
	rpmuzauto(crop1,&crop);
}

/* Shrink x2 stuff */
static Errcode auto_shrink1(void)
{
Errcode err;
int x,y;
int incx, incy;
unsigned rgbacc[3];
Rgb3 rgb;
register int cix;
register UBYTE *ctab;
SHORT dither = vl.ink->dither;

	ctab = (UBYTE *)(vb.pencel->cmap->ctab);
	incx = 2;
	incy = 2;

	if((err = make_bhash()) < 0)
		return(err);

	start_abort_atom();

	for (y=0; y<vb.pencel->height; y+=incy)
	{
		if((err = poll_abort()) < Success)
			break;

		for (x = 0; x < vb.pencel->width; x += incx)
		{
			cix = pj_get_dot(vb.pencel,x,y)*3;
			rgbacc[0] = ctab[cix++];
			rgbacc[1] = ctab[cix++];
			rgbacc[2] = ctab[cix];
			cix = 3*pj_get_dot(vb.pencel,x+1,y);
			rgbacc[0] += ctab[cix++];
			rgbacc[1] += ctab[cix++];
			rgbacc[2] += ctab[cix];
			cix = 3*pj_get_dot(vb.pencel,x,y+1);
			rgbacc[0] += ctab[cix++];
			rgbacc[1] += ctab[cix++];
			rgbacc[2] += ctab[cix];
			cix = 3*pj_get_dot(vb.pencel,x+1,y+1);
			rgbacc[0] += ctab[cix++];
			rgbacc[1] += ctab[cix++];
			rgbacc[2] += ctab[cix];
			rgb.r = (rgbacc[0]>>=2);
			rgb.g = (rgbacc[1]>>=2);
			rgb.b = (rgbacc[2]>>=2);
			pj_put_dot(vb.pencel, bclosest_col(&rgb,COLORS,dither),
				x>>1, y>>1);
		}
	}

	free_bhash();

	pj_set_rect(vb.pencel,vs.inks[0],0,
			 vb.pencel->height>>1,vb.pencel->width, 
			 vb.pencel->height>>1);

	pj_set_rect(vb.pencel,vs.inks[0],
			 vb.pencel->width>>1,0, vb.pencel->width>>1, 
			 vb.pencel->height>>1);

	return(errend_abort_atom(err));
}


void auto_shrink(void)
{
	rpmuzauto(auto_shrink1,NULL);
}


/* Pixelate stuff */
static int quant1(void)
{
Errcode err;
UBYTE *rc;
int x,y,dx,dy;
int ix,iy,zx,zy;
int r,g,b;
int lr,lg,lb;
register UBYTE *c;
int dx2,dy2;
UBYTE rgb[3];

	if (vs.qdx == 1 && vs.qdy == 1)
		return(0);
	if ((err = make_bhash()) < 0)
		return(err);
	rc = (UBYTE *)(vb.pencel->cmap->ctab);
	dx2 = vs.qdx>>1;
	dy2 = vs.qdy>>1;
	for (iy=0;; )
	{
		dy = vb.pencel->height-iy;
		if (dy > vs.qdy)
			dy = vs.qdy;
		for (ix=0;; )
		{
			dx = vb.pencel->width-ix;
			if (dx > vs.qdx)
				dx = vs.qdx;
			zx = ix + dx;
			zy = iy + dy;
			r = b = g = 0;
			for (y=iy; y<zy; y++)
			{
				lr = lb = lg = 0;
				for (x = ix; x<zx; x++)
				{
					c = rc + 3*pj_get_dot(vb.pencel,x,y);
					lr += *c++;
					lg += *c++;
					lb += *c++;
				}
				r += (lr+dx2)/dx;
				g += (lg+dx2)/dx;
				b += (lb+dx2)/dx;
			}
			rgb[0] = (r+dy2)/dy;
			rgb[1] = (g+dy2)/dy;
			rgb[2] = (b+dy2)/dy;
			pj_set_rect(vb.pencel, bclosest_col((Rgb3 *)rgb, 
					 COLORS, FALSE), ix, iy, dx, dy );
			ix += dx;
			if (ix >= vb.pencel->width)
				break;
		}
		iy += dy;
		if (iy >= vb.pencel->height)
			break;
	}
	free_bhash();
	return(0);
}

void quantize(void)
{

	if(!soft_qreq_number(&vs.qdx,1,vb.pencel->width,"quant_wid"))
		return;

	if(!soft_qreq_number(&vs.qdy,1,vb.pencel->height,"quant_hgt"))
		return;

	if (vs.qdx <= 0)
		vs.qdx = 1;
	if (vs.qdy <= 0)
		vs.qdy = 1;
	rpmuzauto(quant1,NULL);
}


/* Expand x2 stuff */

static int halfsies(Rgb3 *a,Rgb3 *b, SHORT dither)
{
UBYTE rgb[3];
int i;

	for (i=0; i<3; i++)
		rgb[i] = (((UBYTE *)a)[i] + ((UBYTE *)b)[i])>>1;
	return(bclosest_col((Rgb3 *)rgb, COLORS, dither) );
}

static int expand1(Rectangle *where)
{
Errcode err;
int xoff, yoff;
int x,y;
int i, j;
UBYTE *s[4];
Rgb3 *ctab;
Rgb3 *aa,*ab,*ba,*bb;
UBYTE me;
int x2,y2;
Rcel *tf;
SHORT dither = vl.ink->dither;


	if((err = alloc_pencel(&tf)) < 0)
		goto error;

	if((err = make_bhash()) < 0)
		goto error;

	xoff = where->x;
	if (xoff > vb.pencel->width/2-1)
		xoff = vb.pencel->width/2-1;
	yoff = where->y;
	if (yoff > vb.pencel->height/2-1)
		yoff = vb.pencel->height/2-1;

	pj_rcel_copy(vb.pencel, tf);
	ctab = vb.pencel->cmap->ctab;

	start_abort_atom();

	for (i=0; i<vb.pencel->height/2; i++)
	{
		if((err = poll_abort()) < Success)
			goto error;

		y = i+yoff;
		for (j=0; j<vb.pencel->width/2; j++)
		{
			x = j+xoff;
			me = pj_get_dot(tf,x,y);
			aa = ctab+me;
			ab = ctab+pj_get_dot(tf,x,y+1);
			ba = ctab+pj_get_dot(tf,x+1,y);
			bb = ctab+pj_get_dot(tf,x+1,y+1);
			x2 = j*2;
			y2 = i*2;
			pj_put_dot(vb.pencel,me,x2,y2);
			pj_put_dot(vb.pencel, halfsies(aa,ab,dither) ,x2,y2+1 );
			pj_put_dot(vb.pencel, halfsies(aa,ba,dither) ,x2+1,y2 );
			pj_put_dot(vb.pencel, halfsies(aa,bb,dither) ,x2+1,y2+1 );
		}
	}
	err = 0;
error:
	free_bhash(); /* ok to do if make_bhash fails */
	pj_rcel_free(tf);
	return(errend_abort_atom(err));
}


void auto_expand(void)
{
	vl.expand_pos.width = vb.pencel->width/2;
	vl.expand_pos.height = vb.pencel->height/2;

	if(rect_in_place(&vl.expand_pos) >= 0)
	{
		if(clip_move_rect(&vl.expand_pos) < Success)
			return;
		vs.expand_x = scale_vscoor(vl.expand_pos.x,vb.pencel->width);
		vs.expand_y = scale_vscoor(vl.expand_pos.y,vb.pencel->height);
	}
	rpmuzauto(expand1,&vl.expand_pos);
}

void auto_setup(Autoarg *aa)

/* undo stuff and common setup stuff before executing auto drawer */
{
	rem_check_tflx_toram();
	flx_clear_olays();
	if(aa->flags & AUTO_HIDEMP)
		hide_mp();
	if(aa->flags & AUTO_USESCEL) /* for now we simply don't allow it */
		fake_push_cel();
	if(!aa->in_preview)
	{
		if(aa->flags & AUTO_UNZOOM)
			unzoom();
	}

	if(aa->flags & AUTO_PUSHMOST)
		push_most();
	else
	{
		if(aa->flags & AUTO_PUSHINKS)
			push_inks();
		if((aa->flags & (AUTO_PUSHCEL|AUTO_USESCEL)) == AUTO_PUSHCEL)
			push_cel();
	}

	if((aa->flags & AUTO_USESCEL) && thecel)
	{
		if(thecel->flif.fd)
			aa->celjmode = JREADONLY;
		else
			aa->celjmode = JUNDEFINED;

		reopen_fcelio(thecel,JREADONLY);
		get_fcelpos(thecel,&aa->cpos);
		aa->cframe = thecel->cd.cur_frame;
	}
}
Errcode auto_restores(Autoarg *aa,Errcode err)

/* restore stuff undone before executing auto drawer */
{
	if((aa->flags & AUTO_USESCEL) && thecel)
	{
		if(err == Err_abort || aa->in_preview)
		{
			put_fcelpos(thecel,&aa->cpos);
			maybe_ref_flicel_pos(thecel);
			seek_fcel_frame(thecel,aa->cframe);
		}
		if(aa->celjmode == JUNDEFINED)
			close_fcelio(thecel);
	}

	if(aa->flags & AUTO_PUSHMOST)
		pop_most();
	else
	{
		if((aa->flags & (AUTO_PUSHCEL|AUTO_USESCEL)) == AUTO_PUSHCEL)
			pop_cel();
		if(aa->flags & AUTO_PUSHINKS)
			pop_inks();
	}
	if(!aa->in_preview)
	{
		if(aa->flags & AUTO_UNZOOM)
			rezoom();
	}
	if(aa->flags & AUTO_USESCEL)
		fake_pop_cel();
	if(aa->flags & AUTO_HIDEMP)
		show_mp();
	set_abort_verify(NULL);
	flx_draw_olays();
	if(!aa->in_preview)
		add_check_tflx_toram();
	see_cmap();  /* some error cases may not sync colormap! */
	return(err);
}

Errcode noask_do_auto(Autoarg *aa, int frame_mode)

/* Come here once user's confirmed they want to do something to many
   frames (or directly if not in time select mode). */
{
Errcode err;
USHORT occolor;
USHORT ocolor8;

	flx_clear_olays();

	if(aa->flags & AUTO_READONLY)
	{
		aa->in_preview = FALSE;
		err = dopreview(aa);
		cleans();
		goto done;
	}

	occolor = vs.ccolor;
	ocolor8 = vs.inks[7];
	get_color_rgb(occolor,vb.pencel->cmap,&aa->auto_rgb);
	get_color_rgb(ocolor8,vb.pencel->cmap,&aa->auto_8rgb);

	switch (frame_mode)
	{
		case DOAUTO_FRAME:
		{
			aa->frames_in_seq = 1;
			aa->cur_frame = 0;
			auto_setup(aa);
			if(aa->flags & AUTO_READONLY)
			{
				err = auto_apply(aa,1,1);
			}
			else
			{
				save_undo();
				err = auto_apply(aa,1,1);
				if(!cmaps_same(vb.pencel->cmap, undof->cmap))
					see_cmap();
				dirties();
			}
			err = auto_restores(aa,err);
			break;
		}
		case DOAUTO_SEGMENT:
			err = dseg(aa);
 			dirty_frame = 0;
			break;
		case DOAUTO_ALL:	/* All Frames */
			err = dall(aa);
 			dirty_frame = 0;
			break;
		default:
			err = Err_bad_input;
			goto error;
	}
	vs.inks[7] = ocolor8;
	vs.ccolor = occolor;
error:
done:
	flx_draw_olays();
	return(softerr(err, "auto_apply"));
}

ErrCode noask_do_auto_time_mode(Autoarg *aa)
/* Do something over time on the current time mode with no menu. */
{
	return noask_do_auto(aa, vs.time_mode);
}


static int clip_t(int t)
/* Make sure a frame index is within bounds of flic.  Chop if not. */
{
	if (t < 0)
		t = 0;
	if (t >= flix.hdr.frame_count)
		t = flix.hdr.frame_count-1;
	return(t);
}

void clip_tseg(void)
/* Make sure both ends of time segment are inside flic. */
{
	vs.start_seg = clip_t(vs.start_seg);
	vs.stop_seg = clip_t(vs.stop_seg);
}

static void useek_cframe(void)
/* seek to frame_ix on undo buffer and then update screen */
{
	fli_abs_tseek(undof, vs.frame_ix);
	zoom_unundo();
}

static void tflx_emessage(Errcode err)
{
	softerr(err, "!%s", "auto_tflx", tflxname);
}

extern char disk_tflx_name[];
#define auto_tflx_name disk_tflx_name

static Errcode dall(Autoarg *aa) 

/* apply function to all frames.  Corrupts undof.
 * avec called to draw something on screen each frame 
 * avec() returns Success if ok ecode if failure (<0) dall will return this
 * ecode */
{
Errcode err;
Jfile new_tflx = JNONE;    /* zeros for error out */
Flx *new_flx = NULL;
void *cbuf = NULL;
int i;
long tsize;
long ssize;
long acc;
long cbufsz;
Rcel *xf;
Boolean do_compress;
Abortbuf abuf;
Boolean abort_atom_nested;

	pstart_abort_atom(&abuf);
	abort_atom_nested = TRUE;
	auto_setup(aa);
	scrub_cur_frame();
	set_abort_verify(auto_abort_verify,aa);

	cbufsz = pj_fli_cbuf_size(vb.pencel->width,vb.pencel->height,
					   vb.pencel->cmap->num_colors);

	/* a rule here is that we only have an avec when we are not
	 * doing overlays */

	if(aa->doing_overlays) /* just building new file from overlays */ 
		xf = undof;
	else if ((err = alloc_pencel(&xf)) < 0)
		goto error;

	/* allocate cbuf here to prevent fragging cause we need it later */

	maybe_push_most();
	err = pj_fli_cel_alloc_cbuf(&cbuf,vb.pencel);
	maybe_pop_most();
	if(err < 0)
		goto error;

	tsize = flix.hdr.frames_in_table*sizeof(Flx);
	if ((new_flx = pj_zalloc(tsize)) == NULL)
		goto nomem_error;

	pj_freez(&cbuf);

	if ((new_tflx = pj_create(auto_tflx_name,JREADWRITE))==JNONE)
	{
		err = pj_ioerr();
		goto newflx_error;
	}

	/* copy all data (header and prefix chunks) 
	 * before index to new flx file */

	flush_tflx();
	if((err = pj_copydata_oset(flix.fd,new_tflx,0,0,flix.hdr.index_oset)) < 0)
		goto newflx_error;
	if(pj_write(new_tflx, new_flx, tsize) < tsize)
	{
		err = pj_ioerr();
		goto newflx_error;
	}

	do_compress = TRUE; /* for non overlay case */

	acc = flix.hdr.index_oset + tsize;
	aa->frames_in_seq = flix.hdr.frame_count;
	for (i=0; i<flix.hdr.frame_count; i++)
	{
		aa->cur_frame = i;
		if((err = poll_abort()) < Success)
			goto errout;

		if(aa->doing_overlays)
		{
			/* note this is for the cel merge overlays scenario which
			 * uses doauto for rendering in non opaque mode rendering
			 * each frame with a poll_abort_atom() so this must be also
			 * see celpaste.c */

			start_abort_atom();
			if(i == 0)
			{
				save_undo();
				if((err = flx_ringseek(undof, vs.frame_ix, 0)) >= Success)
				{
					zoom_unundo();
					do_compress = TRUE; /* brun pretty fast anyway */
				}
			}
			else
			{
				if(flix.overlays[i] != NULL)
				{
					do_compress = TRUE;
					save_undo();
				}
				else
					do_compress = FALSE;

				err = unfli(vb.pencel, i, 1);
			}

			if(err < Success)
				tflx_emessage(err);
			if((err = errend_abort_atom(err)) < Success)
				goto errout;
		}
		else /* doing an avec function */
		{
			if((err = unfli(undof, i, 0)) < 0)
			{
				tflx_emessage(err);
				break; /* an error, but we wish to preserve as much data as
						* possible so we act as if all is ok */
			}
			zoom_unundo();

	    	if( vs.ink_id == tlc_INKID /* GLAZE */
	    		|| vs.ink_id == tsp_INKID) /* GLASS */
			{
				vs.ccolor = closestc(&aa->auto_rgb, 
									 vb.pencel->cmap->ctab, COLORS);
				vs.inks[7] = closestc(&aa->auto_8rgb, 
									  vb.pencel->cmap->ctab, COLORS);
			}

			if(aa->flags & AUTO_USES_UNDO)
			{
				if((cbuf = pj_malloc(cbufsz)) == NULL)
					goto nomem_error;
				pj_get_rectpix(undof,cbuf,0,0,undof->width,undof->height);
			}

			if((err = auto_apply(aa, (i==flix.hdr.frame_count ? 0 : i), 
								 flix.hdr.frame_count)) < 0)
			{
				goto error;
			}

			if(aa->flags & AUTO_USES_UNDO)
			{
				_pj_put_rectpix(undof,cbuf,0,0,undof->width,undof->height);
				pj_freez(&cbuf);
			}

			if(!cmaps_same(vb.pencel->cmap, undof->cmap))
				see_cmap();
		}

		maybe_push_most();
		if ((cbuf = pj_malloc(cbufsz)) == NULL)
		{
			maybe_pop_most();
			goto nomem_error;
		}

		if(!do_compress)
		{
			if((err = read_flx_frame(&flix,cbuf,i)) < 0)
				goto error;
			ssize = ((Fli_frame *)cbuf)->size;
		}
		else if (i == 0)
			ssize = pj_fli_comp_frame1(cbuf,vb.pencel,flix.comp_type);
		else
			ssize = pj_fli_comp_cel(cbuf,xf,vb.pencel,
								 COMP_DELTA_FRAME,flix.comp_type);

		if(pj_i_is_empty_rec(cbuf))
			err = Success; 
		else
		{
			err = pj_write_ecode(new_tflx, cbuf, ssize);
			(new_flx[i]).foff = acc;
			(new_flx[i]).fsize = ssize;
			acc += ssize;
		}

		pj_freez(&cbuf);
		maybe_pop_most();

		if(err < Success)
			goto newflx_error;

		if(!aa->doing_overlays)
			pj_rcel_copy(vb.pencel,xf);
	}

	abort_atom_nested = FALSE;
	if((err = end_abort_atom()) < Success) /* may be aborted in macro */
		goto errout;

	/* reread new first frame */
	if ((cbuf = pj_malloc(cbufsz)) == NULL)
		goto nomem_error;

	if((err = pj_readoset(new_tflx,cbuf,(new_flx[0]).foff,
									  (new_flx[0]).fsize )) < 0)
	{
		goto newflx_error;
	}

	/* and make delta between last and first frame */

	if(aa->doing_overlays) /* undo is xf here */
		save_undo();

	pj_fli_uncomp_frame(vb.pencel, cbuf, 1);
	ssize = pj_fli_comp_cel(cbuf,xf,vb.pencel,
						 COMP_DELTA_FRAME,flix.comp_type);

	if(!pj_i_is_empty_rec(cbuf))
	{
		if((err = pj_writeoset(new_tflx,cbuf,acc,ssize)) < 0)
			goto newflx_error;

		/* allocated cleared 0 if not set */
		(new_flx[flix.hdr.frame_count]).foff = acc;
		(new_flx[flix.hdr.frame_count]).fsize = ssize;
	}

	/* flush index */

	if((err = pj_writeoset(new_tflx,new_flx,flix.hdr.index_oset,tsize)) < 0)
		goto newflx_error;


	pj_freez(&cbuf);
	if(xf != undof)
		pj_rcel_free(xf);
	pj_free(new_flx);
	pj_close(new_tflx);
	close_tflx();
	pj_delete(tflxname);
	pj_rename(auto_tflx_name, tflxname);
	otempflx();
	vs.bframe_ix = 0; /* back frame buffer no good now */
	useek_cframe();
	auto_restores(aa,Success);
 	dirty_strokes += flix.hdr.frame_count;
	dirty_file = TRUE;
	return(Success);

newflx_error:
	err = softerr(err, "!%s", "auto_tflx2", auto_tflx_name);
	goto errout;
nomem_error:
	err = Err_no_memory;
error:
errout:

	if(abort_atom_nested)
		end_abort_atom();

	if(xf != undof)
		pj_rcel_free(xf);
	pj_gentle_free(new_flx);
	pj_gentle_free(cbuf);
	if(new_tflx)
	{
		pj_close(new_tflx);
		pj_delete(auto_tflx_name);
	}

	if(aa->doing_overlays) /* abort or error writing out overlays go all the
							* way around twice to make sure overlays are 
							* refreshed if frame 0 is missing */ 
	{
		restore_with_overlays();
	}
	else
		useek_cframe();
	return(auto_restores(aa,err));
}
Errcode auto_merge_overlays(void) 

/* uses dall with no avec to re compress file with any overlay records
 * merged in */
{
Autoarg aa;

	clear_struct(&aa);
	aa.flags = AUTO_UNZOOM|AUTO_HIDEMP;
	aa.doing_overlays = 1;
	return(dall(&aa));
}

/* Some variable to maintain start and stop of time segment we're
   currently concerned with. */

SHORT tr_r1,tr_r2;
SHORT tr_rdir;
SHORT tr_tix;
SHORT tr_frames;

void find_seg_range(void)

/* Figure out the first and last frame of time segment, how many frames
   are in it, and whether it's reversed or not.  Store result in
   globals above for rest of the world.  */
{
clip_tseg();
tr_frames = vs.stop_seg - vs.start_seg;
if (tr_frames >= 0)
	{
	tr_r1 = vs.start_seg;
	tr_r2 = vs.stop_seg;
	tr_rdir = 1;
	tr_tix = 0;
	}
else
	{
	tr_r1 = vs.stop_seg;
	tr_r2 = vs.start_seg;
	tr_rdir = -1;
	tr_frames = -tr_frames;
	tr_tix = tr_frames;
	}
tr_frames += 1;
}

void find_range(void)

/* Figure out how many frames we want to effect in time-select mode,
   and associated stuff, like what frame to start with. */
{
	switch (vs.time_mode)
	{
		case DOAUTO_FRAME:
			tr_frames = 1;
			tr_r1 = tr_r2 = vs.frame_ix;
			tr_rdir = 1;
			tr_tix = 1;
			break;
		case DOAUTO_SEGMENT:
			find_seg_range();
			break;
		case DOAUTO_ALL:
			tr_frames = flix.hdr.frame_count;
			tr_r1 = 0;
			tr_r2 = flix.hdr.frame_count-1;
			tr_rdir = 1;
			tr_tix = 0;
			break;
	}
}

static int sine_scale(int time_ix, int time_frames)
/* function to help with slow-in/slow-out */
{
int time_scale;

time_scale = rscale_by( TWOPI/2+1, time_ix, time_frames) - TWOPI/4;
time_scale = isin(time_scale);
time_scale /= 2;
time_scale += SCALE_ONE/2;
return(time_scale);
}

/* Figure out time-scale as altered by slow-in/slow-out */
static
calc_ease_in(time_ix, time_frames)
int time_ix, time_frames;
{
int time_scale;

if (time_ix < time_frames/2)
	{
	if (time_ix == 0)
		time_scale = 0;
	else
		{
		time_scale = sine_scale(time_ix, time_frames),
		time_scale = rscale_by(time_scale, 7, 9);
		}
	}
else
	{
	if (time_ix == time_frames)
		time_scale = SCALE_ONE;
	else
		{
		time_scale = rscale_by(sine_scale(time_frames/2,time_frames),7,9)+
		  rscale_by(SCALE_ONE, (time_ix-time_frames/2)*11, time_frames*9);
		}
	}
return(time_scale);
}



int calc_time_scale(int ix, int intween)

/* figure out time-scale for a frame considering ping-pong, reverse,
   slow-in/slow-out, and still */
{
int time_scale;
int time_frames;
int time_ix;

	if (intween <= 1)
		return(SCALE_ONE);
	time_ix = ix;
	if (vs.ado_pong)
	{
		time_frames = (intween+1)/2;
		if (time_ix > time_frames)
			time_ix = time_frames - (time_ix-time_frames);
	}
	else
	{
		time_frames = intween - vs.ado_complete;
		if (vs.ado_reverse)
		{
			time_ix = time_frames - time_ix;
		}
	}
	if (vs.ado_tween)
	{
		if (vs.ado_ease && vs.ado_ease_out)
		{
			/* find index into sine table */
			time_scale = sine_scale(time_ix, time_frames);
		}
		else if (vs.ado_ease)
		{
			time_scale = calc_ease_in(time_ix, time_frames);
		}
		else if (vs.ado_ease_out)
		{
			time_scale = 
				SCALE_ONE - calc_ease_in(time_frames-time_ix,time_frames);
		}
		else
			time_scale = rscale_by(SCALE_ONE, time_ix, time_frames);
	}
	else
		time_scale = SCALE_ONE;
	return(time_scale);
}


Errcode auto_apply(Autoarg *aa, int ix, int intween)
/* go call the guy who diddles the pixels at last! and return its Ecode */
{
Errcode err;

	if((err = (*aa->avec)(aa->avecdat, 
					   ix,intween, calc_time_scale(ix, intween),aa)) < 0)
	{
		return(err);
	}
	zoom_it();

	if(thecel && ((aa->flags & (AUTO_USESCEL|AUTO_NOCELSEEK)) == AUTO_USESCEL))
	{
		err = inc_thecel();
	}
	return(err);
}


Errcode dopreview(Autoarg *aa)

/* Apply rvec just as if were really doing it, but stop short of
   recompressing it into our flic.  Also do the drawing off-screen
   so user only sees result, not intermediate stages This is used
   for both preview and "AUTO_READONLY" render mode */
{
int i;
int oframe_ix;
Rcel *tf;
Rcel *orender;
Rcel_save opic;
Errcode err;
Boolean csame;


	auto_setup(aa);
	scrub_cur_frame();
	if(!aa->in_preview)
		set_abort_verify(auto_abort_verify,aa);

	if ((err = report_temp_save_rcel(&opic, vb.pencel)) < Success)
		return(err);
	oframe_ix = vs.frame_ix;
	find_range();
	save_undo();
	fli_tseek(undof, vs.frame_ix, tr_r1);
	vs.frame_ix = tr_r1;
#ifdef DEBUG
	boxf("tr_r1 %d  tr_r2 %d  tr_rdir %d", tr_r1, tr_r2, tr_rdir);
	boxf("tr_tix %d  tr_frames %d", tr_tix, tr_frames);
#endif /* DEBUG */
	aa->frames_in_seq = tr_frames;
	for (i=0;;i++)
	{
		if((err = poll_abort()) < Success)
			goto OUT;
		aa->cur_frame = i;
		if(NULL != (tf = clone_any_rcel(undof)))
		{
			orender = vb.pencel;
			vb.pencel = tf;
			err = auto_apply(aa, tr_tix, tr_frames);
			vb.pencel = orender;
 			csame = cmaps_same(tf->cmap, vb.pencel->cmap);
 			pj_rcel_copy(tf, vb.pencel);
 			if (!csame)
				see_cmap();
			pj_rcel_free(tf);
		}
		else
		{
			zoom_unundo();
			err = auto_apply(aa, tr_tix, tr_frames);
			see_cmap();
		}
		if(err < 0)
			goto OUT;
		if (i >= tr_frames-1)
			break;
		tr_tix += tr_rdir;
		vs.frame_ix++;
		if((err = unfli(undof,vs.frame_ix,0)) < 0)
			goto OUT;
	}
	if(aa->in_preview)
		wait_click();
OUT:
	vs.frame_ix = oframe_ix;
	report_temp_restore_rcel(&opic, vb.pencel);
	zoom_it();
	save_undo();
	return(auto_restores(aa,err));
}

Errcode do_autodraw(EFUNC avec, void *avecdat)
{
	return(go_autodraw(avec,avecdat,0));
}

Errcode go_autodraw(EFUNC avec, void *avecdat, USHORT flags)
{
Autoarg aa;

	clear_struct(&aa);
	aa.avec = avec;
	aa.avecdat = avecdat;
	aa.flags = flags;
	return(do_auto(&aa));
}

Errcode do_auto(Autoarg *aa)
/* possibly apply a function over many frames after bringing up multimenu */
{
Errcode err;

	flx_clear_olays(); /* undraw cels cursors etc */
	if (vs.multi)
	{
		err = multimenu(aa);
	}
	else
	{
		err = noask_do_auto(aa, DOAUTO_FRAME);
	}
	flx_draw_olays(); /* restore cels and such */
	return(err);
}
