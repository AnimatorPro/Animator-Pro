/* time.c - stuff to insert and delete frames in a FLIC.  Used mostly
   by timemenu.c. */

#include <string.h>
#include "jimk.h"
#include "auto.h"
#include "errcodes.h"
#include "flx.h"
#include "memory.h"
#include "timemenu.h"
#include "zoom.h"

static void delete_first_frames(int frames);
static Errcode delete_middle_frames(int start, int frames);
static Errcode make_frames(int frames);

void qinsert_frames(void)
{
short x;

x = 1;
if (soft_qreq_number(&x,1,100,"insert_after"))
	{
	if (x > 0)
		{
		scrub_cur_frame();
		insert_frames(x, vs.frame_ix);
		}
	}
}


Errcode delete_some(int x)
/* delete X frames from current position forward */
{
Errcode err;
	if(vs.frame_ix == 0 && x == flix.hdr.frame_count)
	{
		kill_seq();
		zoom_it();
		return(Success);
	}
	flx_clear_olays();
	if ((err = scrub_cur_frame()) < Success)
		return(err);
	if (vs.frame_ix == 0)
	{
		delete_first_frames(x);
		if (vs.bframe_ix > x)
			vs.bframe_ix -= x;
		else
			vs.bframe_ix = 0;	/* invalidate back screen cashe */
#ifdef  LATER
		fli_abs_tseek(vb.pencel, 0);
#endif /* LATER */
		save_undo();
	}
	else
	{
		err = delete_middle_frames(vs.frame_ix, x);
		if(vs.bframe_ix >= vs.frame_ix)
			vs.bframe_ix = 0;
		if(vs.frame_ix >= flix.hdr.frame_count) /* we were on the last frame */
		{
			vs.frame_ix = flix.hdr.frame_count-1;
			zoom_unundo();
		}
		else
			save_undo();
	}
	dirties();
	flx_draw_olays();
	return(err);
}
static void delete_range(int start, int count)
{
	flx_clear_olays();
	{
		flx_seek_frame(start);
		delete_some(count);
		flx_draw_olays();
	}
}

void qdelete_frames(void)
{
SHORT x;
SHORT sstart;
SHORT send;

	if(vs.start_seg > vs.stop_seg)
	{
		sstart = vs.stop_seg;
		send = vs.start_seg;
	}
	else
	{
		sstart = vs.start_seg;
		send = vs.stop_seg;
	}

	switch(soft_qchoice(NULL,"!%d%d","frame_del_mu", sstart+1, send+1 ))
	{
		case 0: /* segment */
			clip_tseg();
			delete_range(sstart,send - sstart + 1);
			vs.stop_seg = vs.start_seg = sstart;
			clip_tseg();
			break;
		case 1: /* all but segment */
			if(++send < flix.hdr.frame_count)
				delete_range(send, flix.hdr.frame_count - send );
			if(sstart > 0)
				delete_range(0, sstart);
			vs.stop_seg = vs.start_seg = 0;
			break;
		case 2:
			x = 1;
			if (soft_qreq_number(&x,1,100,"del_from"))
			{
				if (x > 0)
					delete_some(x);
			}
			break;
		default:
			break;
	} /* end switch */
}

static Errcode delete_middle_frames(int start,int frames)
/* delete frames in middle.  Returns with frame after deleted segment in
   vb.pencel, and frame before it in undof */
{
Errcode err;
Fli_frame *cbuf;

	flx_clear_olays();
	/* don't delete past the end mate! */
	if (start + frames >= flix.hdr.frame_count)
		frames = flix.hdr.frame_count - start;

	/* frame before start of deleted segment goes into uf */
	fli_abs_tseek(undof, start-1);
	/* frame after end goes into vf */
	fli_tseek(vb.pencel, vs.frame_ix, start+frames);	
	see_cmap();

	if((err = pj_fli_cel_alloc_cbuf(&cbuf,vb.pencel)) < 0)
		goto OUT;

	/* copy our offset/size table down a notch */
	pj_copy_structure(flix.idx+start+frames, flix.idx+start, 
		(flix.hdr.frame_count+1 - (start+frames))*sizeof(Flx));

	flix.hdr.frame_count -= frames;

	/* compress and rewrite write middle frame */
	pj_fli_comp_cel(cbuf, undof, vb.pencel, COMP_DELTA_FRAME,flix.comp_type);
	if((err = write_flx_frame(&flix,start,cbuf)) < 0)
		goto OUT;

OUT:
	pj_gentle_free(cbuf);
	err = softerr(err,"fli_delete");
	flx_draw_olays();
return(err);
}

static void delete_first_frames(int frames)
{
Errcode err;
Fli_frame *cbuf;

	flx_clear_olays();
	/* check for reasonable frames value */
	if (frames >= flix.hdr.frame_count)
		frames = flix.hdr.frame_count-1;
	if  (frames <= 0)
		return;
	/* at entry vb.pencel is our frame with no menus up.  Will use
	   uf (undo form) for temporary buffer.  */

	/* uf goes to end frame */
	fli_abs_tseek(undof, flix.hdr.frame_count-1);

	/* vb.pencel goes to this frame */
	fli_abs_tseek(vb.pencel, frames);
	see_cmap();

	/* copy our offset/size table down a notch */
	pj_copy_structure(flix.idx+frames, flix.idx, 
		(flix.hdr.frame_count+1 - frames)*sizeof(Flx));

	flix.hdr.frame_count -= frames;

	/* get a compression buffer */
	if((err = pj_fli_cel_alloc_cbuf((Fli_frame **)&cbuf,vb.pencel)) < 0)
		goto error;

	/* compress and write new 1st frame */
	pj_fli_comp_frame1(cbuf,vb.pencel,flix.comp_type);
	if((err = write_flx_frame(&flix,0,cbuf)) < 0)
		goto error;

	/* compress and write new ring frame */
	pj_fli_comp_cel(cbuf, undof, vb.pencel, COMP_DELTA_FRAME,flix.comp_type);

	if((err = write_flx_frame(&flix,flix.hdr.frame_count,cbuf)) < 0)
		goto error;

error:
	pj_gentle_free(cbuf);
	softerr(err,"fli_delete");
	flx_draw_olays();
}


void qmake_frames(void)
{
short x;

	x = flix.hdr.frame_count;
	if(soft_qreq_number(&x,1,100,"set_frames"))
	{
		if (x > 0)
			set_frame_count(x);
	}
}

static Errcode set_flx_length(int frames)
{
Errcode err;

	flx_clear_olays();
	if ((err = scrub_cur_frame()) >= Success)
		err = make_frames(frames);
	flx_draw_olays();
	return(err);
}

void set_frame_count(int x)
{
	if (x > 0)
	{
		if (x >= flix.hdr.frame_count ||
			soft_yes_no_box("!%d", "chop_end_ok", flix.hdr.frame_count - x))
		{
			set_flx_length(x);
		}
	}
}

static Errcode make_frames(int frames)

/* change  the number of frames in this fli.  Will chop off ones at end if
   less, add empty frames (duplicates of last one) if at beginning */
{
Errcode err;

	if (frames == flix.hdr.frame_count)	/* whew! That was easy */
		return(Success);

	flx_clear_olays();
	if (frames > flix.hdr.frame_count)
	{
		err = 
			insert_frames(frames-flix.hdr.frame_count, flix.hdr.frame_count-1);
	}
	else
	{
		err = delete_middle_frames(frames,flix.hdr.frame_count - frames);
		if (vs.frame_ix >= flix.hdr.frame_count)
			vs.frame_ix = flix.hdr.frame_count-1;
		if (frames >= vs.bframe_ix)
			vs.bframe_ix = 0;
		fli_abs_tseek(vb.pencel, vs.frame_ix);
		save_undo();
	}
	dirties();
	flx_draw_olays();
	return(err);
}

Errcode check_max_frames(int count)
{
if (count > MAXFRAMES)
	return(softerr(Err_no_message,"!%d", "fli_max", MAXFRAMES));
else
	return(Success);
}


static Errcode expand_cur_flx(register int new)
{
Errcode err;
register Flx *new_flx;
LONG newbytes;
LONG datoset;
LONG fend;
LONG doff;
int i;

	if (1+new <= flix.hdr.frames_in_table)
		return(Success);
	flx_clear_olays();
	if ((err = check_max_frames(new)) < Success)
		goto error;

	new += 100;	/* don't want to have to do this all the time! */
	if (new > MAXFRAMES)
		new = MAXFRAMES+1; /* +1 for ring frame */

	if ((new_flx = pj_zalloc(new*sizeof(Flx) )) == NULL)
	{
		err = Err_no_memory;
		goto error;
	}
	fend = flx_file_hi();
	datoset = flx_data_offset(&flix);
	copy_mem(flix.idx, new_flx, (flix.hdr.frame_count+1)*sizeof(Flx));
	pj_free(flix.idx);
	flix.idx = new_flx;
	newbytes = (new - flix.hdr.frames_in_table)*sizeof(Flx);

	doff = datoset + newbytes;

	/* for that little special case where index is growing so large
	 * start of new data is beyond end of existing data make sure file is
	 * big enough for seek to data start */

	if(fend <= doff) 
	{
		if((err = pj_write_zeros(flix.fd, fend, doff - fend + 1)) < Success)
			goto error;
	}

	if((err = copy_in_file(flix.fd, fend-datoset, datoset, doff)) < Success)
	  	goto error;

	i = flix.hdr.frame_count;
	while(i-- >= 0) /* adjust offsets of moved data */
	{
		if(new_flx->fsize)
			new_flx->foff += newbytes;
		++new_flx;
	}
	flix.hdr.frames_in_table = new;
	err = Success;
error:
	err = softerr(err,"tflx_expand");
	flx_draw_olays();
	return(err);
}

Errcode insert_frames(int count, /* number of frames to insert */
					  int where) /* frame index to insert frames after */
{
Errcode err;
Flx *open_part;

	flx_clear_olays();
	/* expand frames_in_table if necessary */
	if ((err = expand_cur_flx(count+flix.hdr.frame_count)) < 0)
		goto error;

	++where; /* set to index of frame after */
	open_part = flix.idx + where;
	back_copy_mem(open_part, open_part+count, 
				  (flix.hdr.frame_count+1-where) * sizeof(Flx) );
	clear_mem(open_part, count*sizeof(Flx));
	flix.hdr.frame_count+=count;

	if(where < vs.bframe_ix)
		vs.bframe_ix += count;	/* update back screen cashe index */
	dirties();
	err = Success;
error:
	flx_draw_olays();
	return(err);
}

