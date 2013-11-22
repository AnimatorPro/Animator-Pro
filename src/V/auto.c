
/* auto.c - This file contains the basic methods for doing an operation
   over multiple frames.  The key function is doauto(rvec).  Rvec is
   a function which is called with three parameters:
   		the first is which of the multiple frames it dealing with
		the second is how many frames total there are to deal with
		the last is a scaled percentage indicating how far along
		    in the sequence of frames it is.  (0 for first frame,
			SCALE_ONE (or 1<<14 literally) for the last frame.)
   Rvec is expected to do some transformation on the screen *render_form.
   Rvec also has access to the screen uf (the undo buffer) which will
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
   */


#include "jimk.h"
#include "fli.h"
#include "commonst.h"
#include "auto.str"

extern char new_tflx_name[];
extern WORD x_0,y_0,x_1,y_1;

static UBYTE auto_rgb[3], auto_8rgb[3];

extern dseg();


/* Free up lots of memory by swapping out the world to disk.  Then
   doauto. */

static 
rpmuzauto(what)
Vector what;
{
unzoom();
push_most();
uzauto(what);
pop_most();
rezoom();
}

pmhmpauto(what)
Vector what;
{
push_most();
hmpauto(what);
pop_most();
}


/* Hide menus before do auto */
hmpauto(what)
Vector what;
{
hide_mp();
uzauto(what);
draw_mp();
}

/* Free zoom buffer before doauto */
uzauto(what)
Vector what;
{
unzoom();
doauto(what);
rezoom();
}


/* Clear screen rvec */
static
cpic1()
{
color_form(render_form,vs.inks[0]);
return(1);
}

clear_pic()
{
rpmuzauto(cpic1);
}

/* Blue numbers rvec */
static
blue_num1(ix)
WORD ix;
{
char buf[16];

sprintf(buf, "%4d\n", ix+1);
gtext(buf,XMAX-4*CH_WIDTH,0,vs.inks[1]);
return(1);
}

auto_blue_nums()
{
unzoom();
push_most();
dauto(blue_num1,2);
pop_most();
rezoom();
}


/* Trails stuff */
static Vscreen *tscreen;
static char ttype;
static int tcount;

static
trail1()
{
int ink;

if (tcount)
	{
	switch (ttype)
		{
		case 1:
			transpblit(tscreen, 0, 0, vs.tint_percent);
			copy_form(render_form, tscreen);
			break;
		case 2:
		case 3:
			ink = (ttype == 2 ? 0 : vs.inks[0]);
			tblit8(render_form->w, render_form->h, 
				0, 0, render_form->p, render_form->bpr, 
				0, 0, tscreen->p, BPR, ink);
			copy_form(tscreen, render_form);
			break;
		}
	}
tcount++;
return(1);
}

static char *tchoices[] = {
	auto_101 /* "transparent" */,
	auto_102 /* "zero clear" */,
	auto_103 /* "key color clear" */,
	cst_cancel,
	};

auto_trails()
{
tcount = 0;
unzoom();
push_most();
if ((tscreen = alloc_screen()) == NULL)
	return(0);
if ((ttype = qchoice(auto_105 /* "Type of trail effect" */, tchoices, Array_els(tchoices)))
	!= 0)
	{
	fli_abs_tseek(tscreen,0);
	dauto(trail1,2);
	}
free_screen(tscreen);
pop_most();
rezoom();
}

/* Greys only stuff */
static
grey1()
{
unsigned char ttable[256];
int i;
unsigned char *rgb;

rgb = render_form->cmap;
for (i=0; i<COLORS; i++)
	{
	ttable[i] = (rgb[0] + rgb[1] + rgb[2] + 1)/3;
	rgb+=3;
	}
rgb = render_form->cmap;
for (i=0; i<64; i++)
	{
	*rgb++ = i;
	*rgb++ = i;
	*rgb++ = i;
	}
for (i=64; i<COLORS; i++)
	{
	*rgb++ = 63;
	*rgb++ = 0;
	*rgb++ = 0;
	}
xlat(ttable, render_form->p, 64000);
return(1);
}

greys_only()
{
rpmuzauto(grey1);
}

/* Lace and Engrave stuff */
#define LBSZ (2*sizeof(WORD)*(XMAX+2))
#define BIGC (63*3)

static engrave;	/* is it engrave or dither? */

static
dither1()
{
WORD *lb[2];
int rt;
int i, j, bix;
int x,y;
PLANEPTR p;
PLANEPTR color;
PLANEPTR cmap;
int c;
int s;
int err;
WORD *tp, *np;

if ((lb[0] = begmem(LBSZ)) == NULL)
	return(0);
zero_structure(lb[0], LBSZ);
lb[1] = lb[0]+XMAX+2;
bix = 0;
p = render_form->p;
cmap = render_form->cmap;
for (y=0; y<YMAX; y++)
	{
	tp = lb[bix]+1;
	bix = 1-bix;
	np = lb[bix]+1;
	np[0] = 0;
	np[1] = 0;
	for (x=0; x<XMAX; x++)
		{
		c = *tp++;
		color = cmap + *p * 3;
		c += *color++;
		c += *color++;
		c += *color++;
		if (c>=BIGC/2)
			{
			*p++ = 1;
			err = c - BIGC;
			}
		else
			{
			*p++ = 0;
			err = c;
			}
		if (engrave)	/* originally a bug but I liked it */
			{
			c = (err+2)/4;	/* rounded 1/4ths */
			*tp += c;
			*(np-1) = c;
			*np++ = c;
			*np = err - c - c - c;
			}
		else
			{
			c = (3*err+4)/8;	/* rounded 3/8ths */
			*tp += c;
			*np++ += c;
			*np = err - c - c;
#ifdef FS
			c = (err+2)/4;	/* rounded 1/4ths */
			*tp += c;
			*(np-1) += c;
			*np++ += c;
			*np = err - c - c - c;
#endif FS
			}
		}
	}
copy_bytes(pure_black, cmap, 3);
copy_bytes(pure_white, cmap+3, 3);
freemem(lb[0]);
return(1);
}

static
auto_de()
{
rpmuzauto(dither1);
}

auto_engrave()
{
engrave = 1;
auto_de();
}

auto_dither()
{
engrave = 0;
auto_de();
}

/* Put alt stuff */
static
put_alt1()
{
copy_form(alt_form, render_form);
return(1);
}

auto_put()
{
if (alt_form)
	{
	push_cel();
	uzauto(put_alt1);
	pop_cel();
	}
}

/* Apply ink stuff */
static
render_set1()
{
return(render_box(0,0,XMAX-1,YMAX-1));
}

auto_set()
{
uzauto(render_set1);
}

/* Crop stuff */
static
crop1()
{
WORD omode, ocolor;

omode = vs.draw_mode;
ocolor = vs.ccolor;
vs.draw_mode = 0;
vs.ccolor = vs.inks[0];
render_box(0,0,vf.w-1,y_0-1);
render_box(0,y_1+1,vf.w-1,vf.h-1);
render_box(0,y_0,x_0-1,y_1);
render_box(x_1+1,y_0,vf.w-1,y_1);
vs.draw_mode = omode;
vs.ccolor = ocolor;
return(1);
}

crop_video()
{
save_undo();
if (cut_out())
	{
	swap_box();
	rpmuzauto(crop1);
	}
}

/* Shrink x2 stuff */
static
auto_shrink1()
{
int x,y;
int incx, incy;
unsigned rgbacc[3];
UBYTE rgb[3];
register int cix;
register UBYTE *cmap, *p;

cmap = render_form->cmap;
p = render_form->p;
incx = 2;
incy = 2;

if (!make_bhash())
	return(0);
for (y=0; y<YMAX; y+=incy)
	for (x = 0; x < XMAX; x += incx)
		{
		cix = 3*getd(p,x,y);
		rgbacc[0] = cmap[cix++];
		rgbacc[1] = cmap[cix++];
		rgbacc[2] = cmap[cix];
		cix = 3*getd(p,x+1,y);
		rgbacc[0] += cmap[cix++];
		rgbacc[1] += cmap[cix++];
		rgbacc[2] += cmap[cix];
		cix = 3*getd(p,x,y+1);
		rgbacc[0] += cmap[cix++];
		rgbacc[1] += cmap[cix++];
		rgbacc[2] += cmap[cix];
		cix = 3*getd(p,x+1,y+1);
		rgbacc[0] += cmap[cix++];
		rgbacc[1] += cmap[cix++];
		rgbacc[2] += cmap[cix];
		rgb[0] = (rgbacc[0]>>=2);
		rgb[1] = (rgbacc[1]>>=2);
		rgb[2] = (rgbacc[2]>>=2);
		cdot(p, x>>1, y>>1,bclosest_col(rgb,COLORS));
		}
free_bhash();
cblock(p,0,YMAX>>1,XMAX, YMAX>>1, vs.inks[0]);
cblock(render_form->p,XMAX>>1,0, XMAX>>1, YMAX>>1, vs.inks[0]);

return(1);
}


auto_shrink()
{
rpmuzauto(auto_shrink1);
}


/* Pixelate stuff */
static
quant1()
{
PLANEPTR rp, rc;
int x,y,dx,dy;
int ix,iy,zx,zy;
int r,g,b;
int lr,lg,lb;
register UBYTE *c;
int dx2,dy2;
UBYTE rgb[3];

if (vs.qdx == 1 && vs.qdy == 1)
	return(1);
if (!make_bhash())
	return(0);
rp = render_form->p;
rc = render_form->cmap;
dx2 = vs.qdx>>1;
dy2 = vs.qdy>>1;
for (iy=0;; )
	{
	dy = YMAX-iy;
	if (dy > vs.qdy)
		dy = vs.qdy;
	for (ix=0;; )
		{
		dx = XMAX-ix;
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
				c = rc + 3*getd(rp,x,y);
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
		cblock(render_form->p, ix, iy, dx, dy, bclosest_col(rgb, COLORS) );
		ix += dx;
		if (ix >= XMAX)
			break;
		}
	iy += dy;
	if (iy >= YMAX)
		break;
	}
free_bhash();
return(1);
}

quantize()
{
if (qreq_number(auto_106 /* "Width of quantization unit?" */, &vs.qdx, 1, XMAX))
	if (qreq_number(auto_107 /* "Height of quantization unit?" */, &vs.qdy, 1, YMAX))
		{
		if (vs.qdx <= 0)
			vs.qdx = 1;
		if (vs.qdy <= 0)
			vs.qdy = 1;
		rpmuzauto(quant1);
		}
}


/* Expand x2 stuff */

static
halfsies(a,b)
PLANEPTR a,b;
{
UBYTE  rgb[3];
int i;

for (i=0; i<3; i++)
	{
	rgb[i] = (*a++ + *b++)>>1;
	}
return(bclosest_col(rgb, COLORS) );
}

static
expand1()
{
int xoff, yoff;
int x,y;
int i, j;
UBYTE *s[4];
PLANEPTR d, p, cmap;
PLANEPTR aa,ab,ba,bb;
UBYTE me;
int x2,y2;
Vscreen *tf;

if ((tf = alloc_screen()) != NULL)
	{
	if (!make_bhash())
		return(0);
	xoff = vs.zoomx;
	if (xoff > XMAX/2-1)
		xoff = XMAX/2-1;
	yoff = vs.zoomy;
	if (yoff > YMAX/2-1)
		yoff = YMAX/2-1;
	copy_form(render_form, tf);
	p = tf->p;
	d = render_form->p;
	cmap = render_form->cmap;
	for (i=0; i<YMAX/2; i++)
		{
		y = i+yoff;
		for (j=0; j<XMAX/2; j++)
			{
			x = j+xoff;
			me = getd(p,x,y);
			aa = 3*me+cmap;
			ab = 3*getd(p,x,y+1)+cmap;
			ba = 3*getd(p,x+1,y)+cmap;
			bb = 3*getd(p,x+1,y+1)+cmap;
			x2 = j*2;
			y2 = i*2;
			cdot(d,x2,y2,me);
			cdot(d,x2,y2+1,halfsies(aa,ab) );
			cdot(d,x2+1,y2,halfsies(aa,ba) );
			cdot(d,x2+1,y2+1,halfsies(aa,bb) );
			}
		}
	free_screen(tf);
	free_bhash();
	return(1);
	}
else
	return(0);
}


auto_expand()
{
rpmuzauto(expand1);
}


/* Come here once user's confirmed they want to do something to many
   frames (or directly if not in time select mode). */
dauto(rvec, frame_mode)
Vector rvec;
int frame_mode;
{
int occolor;
int ocolor8;

occolor = vs.ccolor;
ocolor8 = vs.inks[7];
copy_bytes(render_form->cmap+3*occolor, auto_rgb, 3);
copy_bytes(render_form->cmap+3*ocolor8, auto_8rgb, 3);
switch (frame_mode)
	{
	case 0:
		save_undo();
		auto_apply(rvec,1,1);
		see_cmap();
		dirties();
		break;
	case 1: /* Segment */
		if (dseg(rvec))
			stroke_count(tr_frames);
		cleans();
		break;
	case 2:	/* All Frames */
		if (dall(rvec))
			stroke_count(fhead.frame_count);
		cleans();
		break;
	}
vs.inks[7] = ocolor8;
vs.ccolor = occolor;
}



/* Make sure a frame index is within bounds of flic.  Chop if not. */
static
clip_t(t)
int t;
{
if (t < 0)
	t = 0;
if (t >= fhead.frame_count)
	t = fhead.frame_count-1;
return(t);
}

/* Make sure both ends of time segment are inside flic. */
clip_tseg()
{
vs.start_seg = clip_t(vs.start_seg);
vs.stop_seg = clip_t(vs.stop_seg);
}

/* seek to frame_ix on undo buffer and then update screen */
static
useek_cframe()
{
fli_abs_tseek(&uf, vs.frame_ix);
copy_form(&uf, &vf);
see_cmap();
zoom_it();
}


/* apply function to all frames.  Corrupts uf.p */
static
dall(rvec)
Vector rvec;	/* called to draw something on vscreen each frame */
{
int new_tflx;
Flx *new_flx;
long acc;
int i;
long tsize;
long ssize;
char *cbuf;
Vscreen *xf;
int occolor;


if (!scrub_cur_frame())
	return(0);
if ((xf = alloc_screen()) == NULL)
	return(0);
tsize = fhead.frames_in_table*sizeof(Flx);
if ((new_flx = begmemc((unsigned)tsize)) == NULL)
	{
	free_screen(xf);
	return(0);
	}
if ((new_tflx = jcreate(new_tflx_name))==0)
	{
	free_screen(xf);
	freemem(new_flx);
	cant_create(new_tflx_name);
	return(0);
	}
fhead.type = FLIX_MAGIC;
if (jwrite(new_tflx, &fhead, (long)sizeof(fhead)) < sizeof(fhead))
	{
	goto NEWTRUNC;
	}
if (jwrite(new_tflx, &vs, (long)sizeof(vs)) < sizeof(vs))
	{
	goto NEWTRUNC;
	}
if (jwrite(new_tflx, new_flx, tsize) < tsize)
	{
	goto NEWTRUNC;
	}
acc = FLX_OFFSETS + tsize;
for (i=0; i<fhead.frame_count; i++)
	{
	if (check_abort(i, fhead.frame_count) )
		goto BADOUT;
	if (!unfli(&uf, i, 0))
		break;
	copy_form(&uf,render_form);
	see_cmap();
	if (vs.draw_mode == 4 || vs.draw_mode == 6)	/* translucent, transparent */
		{
		vs.ccolor = closestc(auto_rgb, render_form->cmap, COLORS);
		vs.inks[7] = closestc(auto_8rgb, render_form->cmap, COLORS);
		}
	if (!auto_apply(rvec, (i==fhead.frame_count ? 0 : i), fhead.frame_count))
		goto BADOUT;
	if (!maybe_push_most())
		goto BADOUT;
	if ((cbuf = lbegmem(CBUF_SIZE)) == NULL)
		{
		maybe_pop_most();
		goto BADOUT;
		}
	if (i == 0)
		ssize = fli_comp1(cbuf,render_form->p,render_form->cmap);
	else
		ssize = fli_comp_frame(cbuf,xf->p,xf->cmap,
			render_form->p,render_form->cmap,FLI_LC);
	if (jwrite(new_tflx, cbuf, ssize) < ssize)
		{
		freemem(cbuf);
		maybe_pop_most();
		goto NEWTRUNC;
		}
	freemem(cbuf);
	maybe_pop_most();
	(new_flx[i]).foff = acc;
	(new_flx[i]).fsize = ssize;
	acc += ssize;
	copy_form(render_form,xf);
	}
/* reread new first frame */
if ((cbuf = lbegmem(CBUF_SIZE)) == NULL)
	goto BADOUT;
jseek(new_tflx, (new_flx[0]).foff, 0);	/* seek to first frame */
if (jread(new_tflx, cbuf, (new_flx[0]).fsize) <
	(new_flx[0]).fsize )
	{
	truncated(new_tflx_name);
	freemem(cbuf);
	goto BADOUT;
	}
/* and make delta between last and first frame */
uncompfli(render_form, cbuf, 1);
jseek(new_tflx, acc, 0);	/* seek to last frame */
ssize = fli_comp_frame(cbuf,xf->p,xf->cmap,
	render_form->p,render_form->cmap,FLI_LC);
if (jwrite(new_tflx, cbuf, ssize) < ssize)
	{
	freemem(cbuf);
	goto NEWTRUNC;
	}
(new_flx[fhead.frame_count]).foff = acc;
(new_flx[fhead.frame_count]).fsize = ssize;
freemem(cbuf);

GOODOUT:
jseek(new_tflx, FLX_OFFSETS, 0);
if (jwrite(new_tflx, new_flx, tsize) < tsize)
	goto NEWTRUNC;
free_screen(xf);
freemem(new_flx);
jclose(new_tflx);
close_tflx();
jdelete(tflxname);
jrename(new_tflx_name, tflxname);
loaded_screen = 0;
otempflx();
vs.bframe_ix = 0; /* back frame buffer no good now */
useek_cframe();
return(1);

NEWTRUNC:
truncated(new_tflx_name);
BADOUT:
free_screen(xf);
freemem(new_flx);
jclose(new_tflx);
jdelete(new_tflx_name);
useek_cframe();
return(0);
}

/* Some variable to maintain start and stop of time segment we're
   currently concerned with. */
int tr_r1,tr_r2;
int tr_rdir;
int tr_tix;
int tr_frames;

/* Figure out the first and last frame of time segment, how many frames
   are in it, and whether it's reversed or not.  Store result in
   globals above for rest of the world.  */
find_seg_range()
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

/* Figure out how many frames we want to effect in time-select mode,
   and associated stuff, like what frame to start with. */
find_range()
{
switch (vs.time_mode)
	{
	case 0:
		tr_frames = 1;
		tr_r1 = tr_r2 = vs.frame_ix;
		tr_rdir = 1;
		tr_tix = 1;
		break;
	case 1:
		find_seg_range();
		break;
	case 2:
		tr_frames = fhead.frame_count;
		tr_r1 = 0;
		tr_r2 = fhead.frame_count-1;
		tr_rdir = 1;
		tr_tix = 0;
		break;
	}
}

/* function to help with slow-in/slow-out */
static
sine_scale(time_ix, time_frames)
int time_ix, time_frames;
{
int time_scale;

if (time_ix == 0)
	return(0);
else if (time_ix == time_frames)
	return(SCALE_ONE);
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

/* figure out time-scale for a frame considering ping-pong, reverse,
   slow-in/slow-out, and still */
calc_time_scale(ix, intween)
int ix, intween;
{
int time_scale;
int time_frames;
int time_ix;
int temp;

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


/* go call the guy who diddles the pixels at last! */
auto_apply(rvec, ix, intween)
Vector rvec;
int ix, intween;
{
#ifdef DEBUG
	{
	char buf[80];
	sprintf(buf, "auto_apply %d of %d", ix, intween);
	continu_line(buf);
	}
#endif /* DEBUG */
return((*rvec)(ix,intween, calc_time_scale(ix, intween)));
}


/* Apply rvec just as if were really doing it, but stop short of
   recompressing it into our flic.  Also do the drawing off-screen
   so user only sees result, not intermediate stages */
dopreview(rvec)
Vector rvec;
{
int i;
int oframe_ix;
Vscreen *tf;
int ok;

if (!push_screen())
	{
	noroom();
	return;
	}
oframe_ix = vs.frame_ix;
find_range();
copy_form(&vf, &uf);
if (tr_frames > 1)
	scrub_cur_frame();
fli_tseek(&uf, vs.frame_ix, tr_r1);
vs.frame_ix = tr_r1;
for (i=0;;i++)
	{
	check_input();
	if (key_hit || RJSTDN)
		{
		goto OUT;
		}
	if ((tf = alloc_screen()) != NULL)
		{
		copy_form(&uf, tf);
		exchange_words(render_form, tf, sizeof(*tf)/sizeof(WORD) );
		make_dw();
		ok = auto_apply(rvec, tr_tix, tr_frames);
		copy_form(render_form, tf);
		exchange_words(render_form, tf, sizeof(*tf)/sizeof(WORD) );
		make_dw();
		see_cmap();
		free_screen(tf);
		}
	else
		{
		copy_form(&uf, &vf);
		ok = auto_apply(rvec ,tr_tix, tr_frames);
		see_cmap();
		}
	if (!ok)
		break;
	if (i >= tr_frames-1)
		break;
	tr_tix += tr_rdir;
	vs.frame_ix++;
	if (!unfli(&uf,vs.frame_ix,0))
		{
		goto OUT;
		}
	}
wait_click();
OUT:
vs.frame_ix = oframe_ix;
pop_screen();
save_undo();
}

#ifdef OLD
dopreview(rvec)
Vector rvec;
{
if (vs.time_mode == 0)
	{
	save_undo();
	auto_apply(rvec,1,1);
	see_cmap();
	wait_click();
	unundo();
	see_cmap();
	}
else
	{
	preview_many(rvec);
	}
}
#endif OLD

/* Apply a function over many frames */
doauto(rvec)
Vector rvec;
{
if (vs.multi)
	{
	multimenu(rvec);
	check_dfree();
	}
else
	{
	dauto(rvec, 0);
	return;
	}
see_cmap();
}
