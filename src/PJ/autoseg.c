/* autoseg.c - apply a graphics transformation to time segment.  See also
   auto.c, especially function 'dall(rvec)' */


#include "errcodes.h"
#include "jimk.h"
#include "fli.h"
#include "auto.h"

extern Boolean auto_abort_verify();
extern long ff_tflx();

Errcode dseg(Autoarg *aa)

/* Basically repeat rvec to each frame
   in time segment.  Calculate delta between frame before start of time
   segment and the (changed) 1st frame of segment, then store deltas
   between successive changed frames.  Calculate delta between last
   changed frame and frame after end of time segment.  Then go mark all
   the frames in the old time segment as free so space will be reused
   in temporary indexed frame file. 
   
   aa->avec() is assumed to return Success if ok < Success if error */
{
Errcode err = Success;
Rcel *xf;
Fli_frame *cbuf;
int i;
int oframe_ix;
int start;
long size, offset;
Flx *lflx;
int ringit;
long cbufsz;
Abortbuf abuf;

	find_seg_range();		/* set segment start-stop globals */
	pstart_abort_atom(&abuf);
	auto_setup(aa);
	scrub_cur_frame();
	set_abort_verify(auto_abort_verify,aa);
	cbufsz = pj_fli_cbuf_size(vb.pencel->width,vb.pencel->height,
					   vb.pencel->cmap->num_colors);

	oframe_ix = vs.frame_ix;

	ringit = 0;
	if((alloc_pencel(&xf)) >= 0)
	{
		if ((lflx = begmem((tr_frames+1)*sizeof(Flx) )) != NULL)
		{
			start = wrap_frame(tr_r1 - 1);
			pj_rcel_copy(vb.pencel, xf);
			fli_tseek(xf, vs.frame_ix, start);
			pj_rcel_copy(xf, undof);
			aa->frames_in_seq = tr_frames;
			for (i=0;i<=tr_frames;i++)
			{
				aa->cur_frame = i;
				if(poll_abort() < Success)
					goto GETOUT;
				start = wrap_frame(start+1);
				if (unfli(undof,start,0) < Success)
					goto GETOUT;
				pj_rcel_copy(undof, vb.pencel);
				see_cmap();
				if (i != tr_frames)
					if((err = auto_apply(aa, tr_tix, tr_frames)) < 0)
						goto GETOUT;
				tr_tix += tr_rdir;
				maybe_push_most();
				if ((cbuf = begmem(cbufsz)) == NULL)
				{
					maybe_pop_most();
					goto GETOUT;
				}
				if (start == 0)
				{
					size = pj_fli_comp_frame1(cbuf,vb.pencel,flix.comp_type);
					ringit = 1;
				}
				else
					size = pj_fli_comp_cel(cbuf,xf,vb.pencel,COMP_DELTA_FRAME,
										flix.comp_type);

				offset = ff_tflx(size, i, lflx);

				if(offset < 0) /* error ! */
					offset = 0;

				if(!pj_i_is_empty_rec(cbuf))
				{
					if((err = pj_writeoset(flix.fd,cbuf,offset,size))< Success)
					{
						maybe_pop_most();
						pj_free(cbuf);
						goto GETOUT;
					}
					lflx[i].foff = offset;
					lflx[i].fsize = size;
				}
				else /* empty delta */
				{
					lflx[i].foff = 0;
					lflx[i].fsize = 0;
				}
				pj_free(cbuf);
				maybe_pop_most();
				pj_rcel_copy(vb.pencel, xf);
			}
			pj_copy_structure(lflx, flix.idx+tr_r1, (tr_frames+1)*sizeof(Flx) );
GETOUT:

			flush_tflx();
			vs.bframe_ix = 0;	/* disable back frame buffer */
			if (ringit)
			{
				if((cbuf = begmem(cbufsz)) != NULL)
				{
					if(ring_tflx(cbuf) < 0)
						soft_continu_box("no_troom");
					pj_free(cbuf);
				}
			}
			fli_abs_tseek(undof, oframe_ix);
			vs.frame_ix = oframe_ix;
			zoom_unundo();
			pj_free(lflx);
		}
		pj_rcel_free(xf);
	}
	err = auto_restores(aa,errend_abort_atom(err));
	if (err >= Success)
	{
		dirty_strokes += tr_frames;
		dirty_file = TRUE;
	}
	return(err); /* note: always successful unles rvec or flic expand fails */
}
