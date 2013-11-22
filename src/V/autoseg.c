
/* autoseg.c - apply a graphics transformation to time segment.  See also
   auto.c, especially function 'dall(rvec)' */


#include "jimk.h"
#include "fli.h"

extern char new_tflx_name[];
extern render_set();
extern WORD x_0,y_0,x_1,y_1;
extern long ff_tflx();

#ifndef SLUFF
extern long ff_tflx(long size, int count, Flx *xpt);
#endif SLUFF

extern UBYTE auto_rgb[3], auto_8rgb[3];

/* given a frame index add or subtract frame count until have a number
   between 0 and frame count */
static
wrap_ix(ix)
int ix;
{
while (ix < 0)
	ix += fhead.frame_count;
while (ix >= fhead.frame_count)
	ix -= fhead.frame_count;
return(ix);
}

/* faster than the old way at least.  Basically repeat rvec to each frame
   in time segment.  Calculate delta between frame before start of time
   segment and the (changed) 1st frame of segment, then store deltas
   between successive changed frames.  Calculate delta between last
   changed frame and frame after end of time segment.  Then go mark all
   the frames in the old time segment as free so space will be reused
   in temporary indexed frame file. */
static
fast_dseg(rvec)
Vector rvec;
{
Vscreen *xf;
char *cbuf;
int i;
int oframe_ix;
int start, stop;
long size, offset;
Flx *lflx;
int ringit;
int status = 0;

oframe_ix = vs.frame_ix;

ringit = 0;
if ((xf = alloc_screen()) != NULL)
	{
	if ((lflx = begmem((tr_frames+1)*sizeof(Flx) )) != NULL)
		{
		start = wrap_ix(tr_r1 - 1);
		copy_form(render_form, xf);
		fli_tseek(xf, vs.frame_ix, start);
		copy_form(xf, &uf);
		for (i=0;i<=tr_frames;i++)
			{
			if (check_abort(i, tr_frames) )
				goto GETOUT;
			start = wrap_ix(start+1);
			if (!unfli(&uf,start,1))
				{
				goto GETOUT;
				}
			copy_form(&uf, render_form);
			see_cmap();
			if (i != tr_frames)
				if (!auto_apply(rvec, tr_tix, tr_frames))
					goto GETOUT;
			tr_tix += tr_rdir;
			maybe_push_most();
			if ((cbuf = lbegmem(CBUF_SIZE)) == NULL)
				{
				maybe_pop_most();
				goto GETOUT;
				}
			if (start == 0)
				{
				size = fli_comp1(cbuf,render_form->p,render_form->cmap);
				ringit = 1;
				}
			else
				size = fli_comp_frame(cbuf,xf->p,xf->cmap,
					render_form->p,render_form->cmap,FLI_LC);
			offset = ff_tflx(size, i, lflx);
			jseek(tflx, offset, 0);
			if (jwrite(tflx, cbuf, size) < size)
				{
				maybe_pop_most();
				freemem(cbuf);
				noroom();
				goto GETOUT;
				}
			lflx[i].foff = offset;
			lflx[i].fsize = size;
			freemem(cbuf);
			maybe_pop_most();
			copy_form(render_form, xf);
			}
		copy_structure(lflx, cur_flx+tr_r1, (tr_frames+1)*sizeof(Flx) );
GETOUT:
		f_tempflx();
		vs.bframe_ix = 0;	/* disable back frame buffer */
		if (ringit)
			{
			if ((cbuf = lbegmem(CBUF_SIZE)) != NULL)
				{
				if (!ring_tflx(cbuf))
					noroom();
				freemem(cbuf);
				}
			}
		fli_abs_tseek(&uf, oframe_ix);
		vs.frame_ix = oframe_ix;
		copy_form(&uf, render_form);
		see_cmap();
		freemem(lflx);
		status = 1;
		}
	free_screen(xf);
	}
return(status);
}


/* apply rvec over time segment */
dseg(rvec)
Vector rvec;	/* a transformation on render_form... */
{
if (scrub_cur_frame())		/* recompress current frame if dirty */
	{
	find_seg_range();		/* set segment start-stop globals */
	return(fast_dseg(rvec));
	}
}



