/* writeflx.c - stuff to serially create a tempflx file from uncompressed 
 * or compressed frames similar to writefli.c but for the tempflx */

#include "memory.h"
#include "errcodes.h"
#include "flx.h"

void copy_fhead_common(Fli_head *sh, Fli_head *dh)
{
	/* copys all but magic, filesize, or user lock fields between 
	 * two fli headers */
	copy_mem(OPTR(sh,sizeof(Chunk_id)), OPTR(dh,sizeof(Chunk_id)),
			  FLIH_COMMONSIZE - sizeof(Chunk_id));
}
Errcode update_flx_path(Flxfile *flx, Fli_id *flid, char *fliname)
{
Errcode err;
Flipath fp;

	if((err = set_flipath(fliname,flid,&fp)) < Success)
		return(err);
	return(pj_writeoset(flx->fd,&fp,flx->hdr.path_oset,fp.id.size));
}
Errcode read_flx_path(Flxfile *flx, Flipath *fp)
{
Errcode err;

	err = pj_readoset(flx->fd,fp,flx->hdr.path_oset,sizeof(*fp));
	if(err >= Success && fp->id.type != FP_FLIPATH)
		err = Err_bad_magic;
	return(err);
}
Errcode create_flxfile(char *path, Flxfile *flx)
/* this will always leave file position at end of header and path record */
{
	clear_mem(flx,sizeof(*flx));
	flx->hdr.type = FLIX_MAGIC;
	flx->comp_type = pj_fli_comp_ani;
	return(pj_i_create(path,(Flifile *)flx));
}
LONG flx_data_offset(Flxfile *flx)

/* returns offset to first frame chunk in flix */
{
	return(flx->hdr.index_oset+(long)sizeof(Flx)*flx->hdr.frames_in_table);
}
static Errcode flx_write_error(Errcode err,char *name)
{
	if(name)
		err = softerr(err,"!%s", "tflx_write1", name);
	return(err);
}
static Errcode write_flxframe(Flxfile *flxf, char *name, void *frame)

/* assumes this is the next record in the tempflx during serial creation 
 * and previous record was written using this */
{
Errcode err;
Flx *flx;
LONG size = ((Fli_frame *)frame)->size;

	if(flxf->hdr.frame_count >= flxf->hdr.frames_in_table)
	{
		err = Err_too_many_frames;
		goto error;
	}

	flx = flxf->idx + flxf->hdr.frame_count;
	
	if(pj_i_is_empty_rec(frame))
	{
		(flx + 1)->foff = flx->foff; /* propogate forward */
		clear_struct(flx);
		size = 0;
	}
	else
	{
		if((err = pj_writeoset(flix.fd,frame,flx->foff,size)) < Success)
			goto error;
		flx->fsize = size;
		(flx + 1)->foff = flx->foff + size;
	}
	++flxf->hdr.frame_count;
	flxf->hdr.size += size;
	return(Success);
error:
	return(flx_write_error(err,name));
}
static Errcode flx_save_frame(char *name, /* name for error report, if NULL no
									    * report */
						   Flxfile *flxf,
					       void *comp_buf,
						   Rcel *last_screen,
						   Rcel *this_screen,
						   SHORT type)
{
	pj_fli_comp_cel(comp_buf, last_screen, this_screen,type,flxf->comp_type);
	return(write_flxframe(flxf,name,comp_buf));
}
static void frame1_prep(Flxfile *flxf, char *name)
{
	clear_mem(flxf->idx,flxf->hdr.frames_in_table*sizeof(Flx));
	flxf->hdr.size = flxf->idx->foff = flx_data_offset(flxf);
	flxf->hdr.frame_count = 0;
}
Errcode write_first_flxframe(char *name, /* name for error reporting 
								     * if NULL no reports */
					    	Flxfile *flxf,
							void *cbuf,
					 		Rcel *frame1)

/* writes first frame of a fli assuming the output file is positioned to 
 * the first frame position */
{
	frame1_prep(flxf,name);
	return(flx_save_frame(name,flxf,cbuf,NULL,frame1,COMP_FIRST_FRAME));
}
Errcode write_first_flxchunk(char *name, /* name for error reporting 
									      * if NULL no reports */
					    	 Flxfile *flxf, Fli_frame *frame )

/* writes frame that is already compressed in Fli_frame */
{
	frame1_prep(flxf,name);
	return(write_flxframe(flxf,name,frame));
}

Errcode write_next_flxframe(char *name, /* name for error reporting 
								      * if NULL no reports */
					    	Flxfile *flxf, void *cbuf,
					 		Rcel *last_screen, Rcel *this_screen)
{
 	return(flx_save_frame(name,flxf,cbuf,
				  		  last_screen, this_screen, COMP_DELTA_FRAME));
}
Errcode write_next_flxchunk(char *name, /* name for error reporting 
									      * if NULL no reports */
					    	Flxfile *flxf, Fli_frame *frame )

/* writes frame that is already compressed in Fli_frame */
{
	return(write_flxframe(flxf,name,frame));
}
static Errcode write_flx_finish(Flxfile *flxf)
{
	return(flush_flx_hidx(flxf,NULL));
}
Errcode write_ring_flxframe(char *name, /* name for error reporting 
								     * if NULL no reports */
					    Flxfile *flxf, void *cbuf,
					 	Rcel *last_screen, Rcel *first_screen)

/* writes final ring frame of a fli file sets size and flushes the header */
{
Errcode err;
int ocount;

	ocount = flxf->hdr.frame_count;
	if((err = flx_save_frame(name,flxf,cbuf,last_screen,
						     first_screen, COMP_DELTA_FRAME)) < 0)
	{
		goto error;
	}
	if((err = write_flx_finish(flxf)) < 0)
		goto error;
	flxf->hdr.frame_count = ocount;
	return(Success);
error:
	return(flx_write_error(err,name));
}
Errcode write_ring_flxchunk(char *name, /* name for error reporting 
									      * if NULL no reports */
					    	  Flxfile *flxf, Fli_frame *frame )

/* writes frame that is already compressed in Fli_frame */
{
Errcode err;
int ocount;

	ocount = flxf->hdr.frame_count;
	if((err = write_flxframe(flxf,name,frame)) < 0)
		goto error;
	if((err = write_flx_finish(flxf)) < 0)
		goto error;

	flxf->hdr.frame_count = ocount;
	return(Success);
error:
	return(flx_write_error(err,name));
}
Errcode write_first_flxblack(char *name,Flxfile *flxf,Rcel *screen)
/* writes a first frame as a black frame for blank flis includes colors */
{
void *cbuf;
Errcode err;

	if((err=pj_fli_alloc_cbuf((void *)&cbuf,16,16,screen->cmap->num_colors))<0)
		goto error;
	pj_fli_comp_rect(cbuf,NULL,screen,NULL,TRUE,COMP_BLACK_FIRST,flxf->comp_type);
	err = write_first_flxchunk(name,flxf,cbuf);
error:
	pj_gentle_free(cbuf);
	return(flx_write_error(err,name));
}
Errcode write_next_flxempty(char *name,Flxfile *flxf,int num_emptys)
/* writes a no change frame as next frame for blank flis */
{
Flx *flx;

	if(flxf->hdr.frame_count >= flxf->hdr.frames_in_table)
		return(flx_write_error(Err_too_many_frames,name));

	flx = flxf->idx + flxf->hdr.frame_count;
	(flx + num_emptys)->foff = flx->foff;
	flxf->hdr.frame_count += num_emptys;
	return(Success);
}
Errcode write_ring_flxempty(char *name,Flxfile *flxf)
{
Fli_frame frame;

	pj_i_get_empty_rec(&frame);
	return(write_ring_flxchunk(NULL,flxf,&frame));
}
