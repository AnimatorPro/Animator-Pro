
/* tempflx.c - stuff that helps manage our main scratch file. */

#include <time.h>
#include "errcodes.h"
#include "fli.h"
#include "menus.h"
#include "jimk.h"
#include "unchunk.h"
#include "picdrive.h"

/* This module copies a single .FLI file to our TEMP.FLX file.
	A FLX file is just a FLI file with frame offsets and configuration
	settings at the start.
	This is sort of a private Vpaint file.  It's loaded on start-up if
	it exists already */


Flxfile flix; /* THE temp flx file */

void close_flx(Flxfile *flx)
{
	pj_gentle_free(flx->idx);
	free_flx_overlays(flx);
	if(flx->fd)
		pj_close(flx->fd);
	clear_mem(flx,sizeof(*flx)); 
	flx->comp_type = pj_fli_comp_ani;
}
static Errcode alloc_flx_index(Flx **flx,int num_entries)
{
	if(NULL == (*flx = pj_zalloc(num_entries*sizeof(Flx))))
		return(Err_no_memory);
	return(0);
}
static Errcode open_flx(char *path, Flxfile *flx,int jmode)

/* opens an existing flx file for read write */
{
LONG err;
long acc;

	clear_struct(flx);
	if ((flx->fd = pj_open(path,jmode)) == JNONE)
		return(pj_ioerr());

	if (pj_read(flx->fd, &flx->hdr, (long)sizeof(Flx_head)) < sizeof(Flx_head))
		goto jio_error;
	if(flx->hdr.type != FLIX_MAGIC)
	{
		err = Err_bad_magic;
		goto error;
	}
	acc = flx->hdr.frames_in_table; 
	if((err = alloc_flx_index(&flx->idx,acc)) < Success)
		goto error;
	acc *= sizeof(Flx);

	if((err = pj_readoset(flx->fd, flx->idx, flx->hdr.index_oset, acc)) < 0)
		goto error;

	flx->comp_type = pj_fli_comp_ani;
	return(Success);
jio_error:
	err = pj_ioerr();
error:
	if(err == Err_eof)
		truncated(path);
	else
		softerr(err, "!%s", "tflx_open", path );
	close_flx(flx);
	return(err);
}
static Errcode flush_flx_index(Flxfile *flx)

/* flushes or writes index of flx file leaves file position at end of index */
{
	return(pj_writeoset(flx->fd, flx->idx, flx->hdr.index_oset, 
					  flx->hdr.frames_in_table*sizeof(Flx)));
}
Errcode flush_flx_hidx(Flxfile *flx)

/* (re-)writes the header, settings chunk, and index of a flx file */
{
LONG err;

	update_flx_id(flx);
	if((err = pj_writeoset(flx->fd,&flx->hdr,0,sizeof(flx->hdr))) < 0)
		goto error;	
	return(flush_flx_index(flx));
error:
	return(err);
}

void flush_tflx(void)
{
	if(!flix.fd)
		return;
	flush_tsettings(FALSE);
	flush_flx_hidx(&flix);
}
void close_tflx(void)
{
	rem_check_tflx_toram(); /* fudgy but this will take it out */
	vs.frame_ix = 0;
	close_flx(&flix);
}
static Errcode new_flx_prefix(Flxfile *flx, Fli_id *flid, char *fliname)

/* creates a new (default) optional prefix chunk for a tempflx file with 
 * the default chunks */
{
Errcode err;
Chunk_id fchunk;

	fchunk.type = FCID_PREFIX;
	fchunk.size = sizeof(fchunk) + sizeof(Flipath);

	if((err = pj_writeoset(flx->fd,&fchunk,
						 sizeof(Flx_head),sizeof(fchunk)) < Success))
	{
		return(err);
	}

	flx->hdr.path_oset = sizeof(Flx_head) + sizeof(fchunk);
	if((err = update_flx_path(flx, flid, fliname)) < 0)
		return(err);
	return(Success);
}
static Errcode empty_flx_start(char *path,Flxfile *flx, int iframes)

/* opens and creates an empty tempflx file to match the size of the current
 * penwndo will install iframes blank frames (empty index) */
{
Errcode err;
long itable;

	if(((unsigned)iframes) > MAXFRAMES)
	{
		err = Err_too_many_frames;
		goto error;
	}

	if(!iframes)
		iframes = 1;
	itable = iframes + 100;
	if(itable > MAXFRAMES+1)
		itable = MAXFRAMES+1;
	else if(itable < 255)
		itable = 255;

	close_tflx();
	if((err = create_flxfile(path,flx)) < 0)
		goto error;
	flx->hdr.frame_count = iframes;
	flx->hdr.width = vb.pencel->width;
	flx->hdr.height = vb.pencel->height;
	flx->hdr.bits_a_pixel = 8; /* vb.pencel->pdepth */
	flx->hdr.frames_in_table = itable;  /* make a blank index... */
	flx->hdr.speed = FLX_DEFAULT_SPEED;

	if((err = alloc_flx_index(&flx->idx,itable)) < Success)
		goto error;

	/* write new default prefix chunk */

	if((err = new_flx_prefix(flx,NULL,NULL)) < 0)
		goto error;

	flx->hdr.index_oset = pj_tell(flx->fd);
	if((err = flush_flx_index(flx)) < 0) /* flush (write) new index */ 
		goto error;

	if((err = flush_flx_index(flx)) < 0) /* flush (write) new index */ 
		goto error;
	return(Success);
error:
	close_flx(flx);
	return(err);
}
Errcode empty_tempflx(int iframes)

/* opens and creates an empty tempflx file to match the size of the current
 * penwndo will install iframes blank frames (empty index) */
{
Errcode err;

	if((err = empty_flx_start(tflxname,&flix,iframes)) < Success)
		goto error;

	/* write out first black frame (note this clears the index) */

	if((err = write_first_flxblack(NULL,&flix,vb.pencel)) < Success)
		goto error;

	if(--iframes > 0) /* create subsequent blank frames */
	{
		if((err = write_next_flxempty(NULL,&flix,iframes)) < Success)
			goto error;
	}
	if((err = write_ring_flxempty(NULL,&flix)) < Success)
		goto error;

	cleans();
	return(Success);
error:
	close_tflx();
	return(softerr(err,"!%s", "tflx_empty", tflxname));
}

Errcode otempflx()
{
Errcode err;

	close_tflx();
	if((err = open_flx(tflxname,&flix,JREADWRITE)) >= Success)
	{
		if((err = reload_tsettings(&vs,NULL)) < Success)
			close_tflx();
	}
	return(err);
}

Errcode open_tempflx(Boolean reload_settings)
{
Errcode err;

	vs.bframe_ix = 0; /* back frame buffer no good now */
	if((err = otempflx()) < Success)
		return(err);
	rethink_settings();
	return(Success);
}

static Errcode create_tflx_start(Flifile *flif,char *fliname,long extra_frames)

/* makes header, prefix chunks, and index for a tflx from another fli 
 * file and an index size. It will leave the file position at the start of the
 * first frame chunk (yet to be written) */
{
Errcode err;
Chunkparse_data pd;

	if((err = create_flxfile(tflxname,&flix)) < Success)
		goto error;

	/* move in common fields defining fli and it's creator id */

	copy_fhead_common(&flif->hdr,&flix.hdr);

	/* allocate new cleared index of size requested */

	flix.hdr.frames_in_table = 
	 			(flif->hdr.frame_count+extra_frames+256) & 0xFFFFFF00;

	if((err = alloc_flx_index(&flix.idx,flix.hdr.frames_in_table)) < Success)
		goto error;

	/** copy any relevant prefix chunks from the fli (most of them) */

	init_chunkparse(&pd,flif->fd,FCID_PREFIX,sizeof(Fli_head),0,0);
	while(get_next_chunk(&pd))
	{
		switch(pd.type)
		{
			case ROOT_CHUNK_TYPE: 
			{
				if((err = copy_parsed_chunk(&pd, flix.fd)) < Success)
					goto error;
				flix.hdr.path_oset = pj_tell(flix.fd);
				if((err = update_flx_path(&flix,&flif->hdr.id,fliname)) < 0)
					goto error;
				break;
			}
			case FP_VSETTINGS: /* ignore anything else */
			case FP_FLIPATH: 
			case FP_FREE:
			default:
				break;
			case FP_CELDATA:
				if(pd.fchunk.size != sizeof(Celdata))
				{
					softerr(Err_unimpl, "!%d%d",
							"tflx_celdata", 
							sizeof(Celdata), pd.fchunk.size );
					break;
				}
		}
	}

	if((err = pd.error) >= Success) /* we matched and parsed a PREFIX chunk */
	{
		/* set size of copied PREFIX chunk and flush */

		flix.hdr.index_oset = pj_tell(flix.fd);

		pd.fchunk.type = FCID_PREFIX;
		pd.fchunk.size = flix.hdr.index_oset - sizeof(Flx_head);
		if((err = pj_writeoset(flix.fd,&pd.fchunk,
							 sizeof(Flx_head),sizeof(Chunk_id))) < Success)
		{
			goto error;
		}
	}
	else if(err == Err_no_chunk)
	{
		if((err = new_flx_prefix(&flix,&flif->hdr.id,fliname)) < 0)
			goto error;
		flix.hdr.index_oset = pj_tell(flix.fd);
	}
	else
		goto error;

	/* set magic flush header, settings, and write index */

	if((err = flush_flx_hidx(&flix)) < Success)
		goto error;

	return(Success);

error:
	return(softerr(err,"!%s", "tflx_header", tflxname));
}
Errcode ring_tflx(Fli_frame *cbuf)
/* Generate a delta between the last frame of flix and the 1st frame of 
 	flix and finish the file */
{
Errcode err;
long size;
int ix;

	ix = flix.hdr.frame_count;
	clear_mem(flix.idx + ix,(flix.hdr.frames_in_table - ix)*sizeof(Flx));
	vs.bframe_ix = 0;	/* invalidate back frame cashe */
	gb_fli_abs_tseek(undof,ix - 1,cbuf);
	gb_fli_abs_tseek(vb.pencel, 0,cbuf);
	see_cmap();
	size = pj_fli_comp_cel(cbuf,undof,vb.pencel,COMP_DELTA_FRAME,flix.comp_type);
	if(pj_i_is_empty_rec(cbuf))
		size = 0;
	if((err = make_flx_record(&flix,ix,cbuf,size,1)) < 0)
		{
		return(err);
		}
	return(flush_flx_index(&flix));
}

static Errcode abort_anim_load(char *name,int frame_count)
{
	if(soft_yes_no_box("!%s%d%d", "load_abort",
				   name,  flix.hdr.frame_count, frame_count))
	{
		return(Err_abort);
	}
	return(Success);
}
static Errcode ring_loaded_anim(char *name,Errcode reason,int frame_count)
/* We come here if have partially loaded a flic, but got an error somewhere
 * along the way.   Try to make up a ring frame.  If this fails back up
 * a bit and try again.  1st failure back up one frame, second back up
 * 2 frames, etc.  If can't make ring frame of 1 frame animation then
 * return error. */
{
Errcode err;
Fli_frame *frame;

	if((err = pj_fli_cel_alloc_cbuf(&frame, vb.pencel)) < Success)
		return(err);

	while(flix.hdr.frame_count > 0) 
	{
		soft_put_wait_box("wait_ringing");
		if((err = ring_tflx(frame)) >= Success)
		{
			/* note this won't report if reason is Success or Err_abort */

			cleanup_wait_box();
			softerr(reason, 
					"!%s%d%d",  "tflx_ring",
					 name, flix.hdr.frame_count, frame_count);
			break;
		}
		--flix.hdr.frame_count;
		reason = err;
	}
	cleanup_wait_box();
	pj_freez(&frame);
	return(err);
}
Errcode fli_to_tempflx(char *name, int extra_frames, Boolean allow_abort)

/* closes old tflx and creates a new tempflx from a fli (file name) 
 * this reports errors will attempt to allow a partial tempflx if 
 * only part is read in */
{
Errcode err;
Flifile flif;
Fli_frame *frame = NULL;

	extra_frames += 100;
	close_tflx();
	cleans();

	if((err = squawk_open_flifile(name,&flif,JREADONLY)) < 0)
		return(err);

	if (flif.hdr.width != vb.pencel->width
		|| flif.hdr.height != vb.pencel->height)
	{
		err = Err_wrong_res;
		goto error;
	}

	if ((err = create_tflx_start(&flif,name,extra_frames)) < Success)
		goto error;

	if((err = pj_fli_cel_alloc_cbuf(&frame, vb.pencel)) < 0)
		goto error;

	if((err = pj_i_read_uncomp1(NULL,&flif,NULL,frame,0)) < Success)
		goto error;

	dirty_frame = !pj_frame_has_pstamp(frame);

	if((err = write_first_flxchunk(NULL,&flix,frame)) < Success)
		goto error;

	for(;;)
	{
		if((err = pj_fli_read_uncomp(NULL,&flif,NULL,frame,0)) < Success)
			goto error;

		if(allow_abort && (poll_abort() < Success))
		{
			if((err = abort_anim_load(name, flif.hdr.frame_count)) < Success)
				break;
		}

		if(flix.hdr.frame_count < flif.hdr.frame_count)
		{
			if((err = write_next_flxchunk(NULL,&flix,frame)) >= Success)
				continue;
		}
		else
		{
			if((err = write_ring_flxchunk(NULL,&flix,frame)) >= Success)
				goto done;
		}
		break;
	}

	pj_freez(&frame);
	if((err = ring_loaded_anim(name,err,flif.hdr.frame_count)) >= Success)
		goto done;

error:
	close_tflx();
done:
	pj_freez(&frame);
	pj_fli_close(&flif);
	return(err);
}
Errcode make_pdr_tempflx(char *pdr_name,char *flicname, Anim_info *ainfo)
{
Errcode err;
Pdr *pd;
Image_file *ifile;
Fli_frame *cbuf = NULL;
int frame_count, frames_left;

	close_tflx();
	if((err = load_pdr(pdr_name, &pd)) < Success)
		return(cant_use_module(err,pdr_name));
	if((err = pdr_open_ifile(pd, flicname, &ifile, ainfo)) < Success)
		goto error;

	if((frame_count = ainfo->num_frames) > MAXFRAMES)
	{
		if(!soft_yes_no_box("!%s%d%d", "max_frames",
					   flicname, frame_count, MAXFRAMES ))
		{
			err = Err_abort;
			goto error;
		}
		frame_count = MAXFRAMES;
	}

	if((err = empty_flx_start(tflxname,&flix,frame_count)) < Success)
		goto error;

	flix.hdr.speed = ainfo->millisec_per_frame;
	if(ainfo->aspect_dy)
	{
		flix.hdr.aspect_dx = ainfo->aspect_dx;
		flix.hdr.aspect_dy = ainfo->aspect_dy;
	}

	if((err = pdr_read_first(ifile,vb.pencel)) < Success)
		goto error;

	if((err = pj_fli_cel_alloc_cbuf(&cbuf, vb.pencel)) < Success)
		goto error;
	err = write_first_flxframe(NULL,&flix,cbuf,vb.pencel);
	pj_freez(&cbuf);
	if(err < Success)
		goto error;

	frames_left = frame_count;
	while(--frames_left)
	{
		if(poll_abort() < Success)
		{
			if((err = abort_anim_load(flicname, frame_count)) < Success)
				break;
		}
		save_undo();
		if((err = pdr_read_next(ifile,vb.pencel)) < Success)
			goto ring_error;
		if((err = pj_fli_cel_alloc_cbuf(&cbuf, vb.pencel)) < 0)
			goto ring_error;
		err = write_next_flxframe(NULL,&flix,cbuf,undof,vb.pencel);
		pj_freez(&cbuf);
		if(err < Success)
			goto ring_error;
	}

	if((err = pj_fli_cel_alloc_cbuf(&cbuf, vb.pencel)) < 0)
		goto ring_error;
	fli_abs_tseek(undof, 0);
	err = write_ring_flxframe(NULL,&flix,cbuf,vb.pencel,undof);
	pj_freez(&cbuf);
	if(err >= Success)
		goto done;

ring_error: /* call backoff and ring routine */

	if((err = ring_loaded_anim(flicname,err,frame_count)) >= Success)
		goto done;
error:
	close_tflx();
done:
	pdr_close_ifile(&ifile);
	free_pdr(&pd);
	flush_tflx();
	return(err);
}
Errcode make_tempflx(char *name,Boolean allow_abort)
{
Errcode err;

	pj_delete(tflxname);
	maybe_push_most();
	if((err = fli_to_tempflx(name,0,allow_abort)) < 0)
		empty_tempflx(1);
	maybe_pop_most();
	return(err);
}
static Errcode set_first_frame(int ix)
{
Errcode err;
Fli_frame *cbuf;
Flx *oftab;
LONG sz, tsz;
int i;

	if(ix >= flix.hdr.frame_count || ix == 0)
		return(Success);

	hide_mp();
	unzoom();
	flx_clear_olays(); /* undraw cels cursors etc */

	soft_put_wait_box("wait_reorder");
	save_undo();

	sz = pj_fli_cel_cbuf_size(vb.pencel);
	tsz = flix.hdr.frames_in_table * sizeof(Flx);

	if((cbuf = pj_zalloc(Max(sz,tsz))) == NULL)
	{
		err = Err_no_memory;
		goto error;
	}

	if((err = gb_fli_tseek(undof, vs.frame_ix, ix, cbuf)) < 0)
		goto error;
	zoom_unundo();

	oftab = (Flx *)cbuf;
	copy_mem(flix.idx,oftab,sz);

	for(i = 1;i <= flix.hdr.frame_count;++i)
	{
		if(++ix > flix.hdr.frame_count)
			ix = 1;
		flix.idx[i] = oftab[ix]; 
	}
	vs.bframe_ix = 0;
	vs.frame_ix = 0;
	pj_fli_comp_frame1(cbuf,vb.pencel,flix.comp_type);
	if((err = write_flx_frame(&flix,0,cbuf)) < 0)
		goto error;
	flush_tflx();

error:
	pj_gentle_free(cbuf);
	err = softerr(err,"fli_reorder");

	flx_draw_olays(); /* restore cels and such */
	rezoom();
	show_mp();
	return(err);
}
void qset_first_frame(void *data)
{
SHORT ix;

	ix = vs.frame_ix + 1;
	if(!soft_qreq_number(&ix,1,flix.hdr.frame_count,"set_frame0"))
		return;
	set_first_frame(ix - 1);
}
void empty_cleared_flx(Pixel color)

/* called from auto if desired to clear whole flx in clear pic do auto */
{
Errcode err;
SHORT oix;
LONG ospeed;
Fli_frame *cbuf;

	hide_mp();
	unzoom();
	push_most();

	/* start with empty tempflx with table large enough for current flx */
	oix = vs.frame_ix;
	ospeed = flix.hdr.speed;
	empty_tempflx(flix.hdr.frame_count);
	flix.hdr.speed = ospeed; /* preserve this */

	pj_set_rast(vb.pencel,color);

	if(color != 0) /* empty already set to color 0 */
	{
		if((err = pj_fli_cel_alloc_cbuf(&cbuf, vb.pencel)) < Success)
			goto error;

		/* make a new first frame if not color zero
		 * and re-write first frame.  All others remain blank,
		 * including ring. */

		pj_fli_comp_frame1(cbuf,vb.pencel,flix.comp_type);
		if((err = write_flx_frame(&flix,0,cbuf)) < Success)
			goto error;
		flush_tflx();
		pj_free(cbuf);
	}
	vs.frame_ix = oix;
	goto done;

error:
	softerr(err,NULL);
	empty_tempflx(1);
	pj_clear_rast(vb.pencel);
	vs.frame_ix = 0;
done:
	vs.bframe_ix = 0; /* kill backframe */
	pop_most();
	rezoom();
	show_mp();
}
