/*****************************************************************************
 * FLILO.C - Rex and PDR interface routines for low-rez FLI writer.
 ****************************************************************************/

#include "errcodes.h"
#include "stdio.h"
#include "picdrive.h"
#include "syslib.h"
#include "filepath.h"
#include "flilo.h"
#include "jfile.h"


static Errcode pj_i_flush_head(Flifile *flif)

/* Updates id and flushes header of a Flifile leaves file offset
 * at end of header */
{
	return(pj_jwriteoset(flif->fd,&flif->hdr,0,sizeof(flif->hdr)));
}

static Errcode pj_i_add_next_rec(Flifile *flif, Fli_frame *frame )
{
Errcode err;
LONG size = ((Fli_frame *)frame)->size;

	if (pj_write(flif->fd, frame, size) < size)
	{
		if((err = pj_ioerr()) == Err_eof)
			err = Err_truncated;

		/* attempt flush of header */
		pj_i_flush_head(flif);
		return(err);
	}
	flif->hdr.size += size;
	++flif->hdr.frame_count;
	return(Success);
}

static Errcode pj_fli_add_frame1( Flifile *flif, void *cbuf, Rcel *frame1)

/* will compress the first frame, from the screen provided, and call
 * fii_add_next_rec() with the resultant record */

{
	pj_fli_comp_cel(cbuf, NULL, frame1, FLI_BRUN);
	flif->hdr.size = pj_tell(flif->fd);
	if(flif->hdr.size < 0)
		return((Errcode)flif->hdr.size);
	flif->hdr.frame_count = 0;
	return(pj_i_add_next_rec(flif,cbuf));
}

static Errcode pj_fli_add_next( Flifile *flif, void *cbuf,
							 Rcel *last_screen, Rcel *this_screen)

/* calculates an intermediate delta record from the two rasters provided
 * and writes it to the output fli file.  Mus be called after a
 * fli_add_frame1() or a previous fli_add_next() */
{
	pj_fli_comp_cel(cbuf, last_screen, this_screen, FLI_LC);
	return(pj_i_add_next_rec(flif,cbuf));
}

static Errcode pj_i_add_ring_rec(Flifile *flif, Fli_frame *frame )
{
Errcode err;
int ocount;

	ocount = flif->hdr.frame_count;
	if((err = pj_i_add_next_rec(flif,frame)) < Success)
		goto error;
	flif->hdr.frame_count = ocount;
	flif->hdr.flags = (FLI_FINISHED | FLI_LOOPED);
	err = pj_i_flush_head(flif);
error:
	return(err);
}

static Errcode pj_fli_add_ring( Flifile *flif, void *cbuf,
							 Rcel *last_screen, Rcel *first_screen)

/* compresses record and writes a final ring frame (delta between last frame
 * and first) of a fli file sets header data and flushes the header. The file
 * is now complete and may be closed, or played. */
{
	pj_fli_comp_cel(cbuf, last_screen, first_screen, FLI_LC);
	return(pj_i_add_ring_rec(flif,cbuf));
}

static Errcode pj_i_add_empty_ring(Flifile *flif)
/* Same as fli_add_ring() but writes a no change frame as the
 * next frame for blank flis */
{
Fli_frame frame;

	memset(&frame,0,sizeof(frame));
	frame.size = sizeof(frame);
	frame.type = FCID_FRAME;
	return(pj_i_add_ring_rec(flif,&frame));
}

static Boolean flow_spec_best_fit(Anim_info *ainfo)
{
	if(ainfo->depth == 8
		 && ainfo->num_frames <= MAXFRAMES
		 && ainfo->width == 320
		 && ainfo->height == 200)
	{
		return(TRUE);
	}

	ainfo->depth = 8;
	if(ainfo->num_frames > MAXFRAMES)
		ainfo->num_frames = MAXFRAMES;
	ainfo->width = 320;
	ainfo->height = 200;
	return(FALSE);
}

static int files_open;

static close_flow_file(Image_file **pifile)
{
Flifile *flif;

	if(NULL == (flif = ((Flifile *)(*pifile))))
		return;

	pj_close(flif->fd);
	free(flif);
	*pifile = NULL;
	--files_open;
}

static Errcode create_flow_file(Pdr *pd, char *path, Image_file **pif,
								 Anim_info *ainfo )
{
Flifile *flif;
Errcode err;

	if(files_open)
		return(Err_too_many_files);

	if(NULL == (*pif = zalloc(sizeof(Flifile))))
		return(Err_no_memory);
	flif = (Flifile *)(*pif);
	++files_open;

	if((flif->fd = pj_create(path,JREADWRITE)) == JNONE)
		goto jio_error;

	flif->ifile.needs_work_cel = TRUE;
	flif->hdr.type = FLIH_MAGIC;
	flif->hdr.width = 320;
	flif->hdr.height = 200;
	flif->hdr.bits_a_pixel = 8;
	flif->hdr.speed = ((((long)ainfo->millisec_per_frame)*70L)+500L)/1000L;

	if((err = pj_i_flush_head(flif)) < Success)
		goto error;

	return(Success);
jio_error:
	err = pj_ioerr();
error:
	close_flow_file(pif);
	return(err);
}
static void pj_freez(void *pt)
{
	if(*((void **)pt) != NULL)
	{
		free(*((void **)pt));
		*((void **)pt) = NULL;
	}
}

static Errcode flow_save_frames(Image_file *ifile,
								Rcel *screen,
								int num_frames,
								Errcode (*seek_frame)(int ix,void *seek_data),
								void *seek_data,
								Rcel *work_screen )
{
Errcode err = Success;
int i;
Fli_frame *cbuf;
Flifile *flif = ((Flifile *)ifile);

	if(screen->width != 320 || screen->height != 200)
		return(Err_wrong_res);

	cbuf = NULL;
	for(i = 0;;++i)
	{
		if(NULL == (cbuf = malloc(CBUF_SIZE)))
		{
			err = Err_no_memory;
			break;
		}
		if(i == 0)
		{
			if((err = pj_fli_add_frame1(flif, cbuf, screen)) < Success)
				break;

			/* if only one frame terminate file and exit */

			if(num_frames <= 1)
			{
				err = pj_i_add_empty_ring(flif);
				break;
			}
		}
		else if(i < num_frames)
		{
			if((err = seek_frame(i,seek_data)) < Success)
				break;
			if((err = pj_fli_add_next(flif,cbuf,work_screen,screen)) < Success)
				break;
		}
		else
		{
			/* if larger than one frame file re-seek to first frame */
			if((err = seek_frame(0,seek_data)) < Success)
				break;
			err = pj_fli_add_ring(flif,cbuf,work_screen,screen);
			break; /* we are done ! */
		}
		pj_freez(&cbuf);
		pj_blitrect(screen,0,0,work_screen,0,0,320,200);
		pj_cmap_copy(screen->cmap, work_screen->cmap);
	}

	pj_freez(&cbuf);
	return(err);
}


/*****************************************************************************
 * REX/Pdr interface stuff...
 ****************************************************************************/

#define HLIB_TYPE_1 AA_STDIOLIB
#define HLIB_TYPE_2 AA_SYSLIB
#define HLIB_TYPE_3 AA_GFXLIB

#include "hliblist.h"

static char title_info[] = "Animator 1.0 320 X 200 FLI format.";

static char long_info[] = RL_KEYTEXT("flilo_info")"Will create files readable"
						  " by low resolution Animator.";

Pdr rexlib_header = {
	{ REX_PICDRIVER, PDR_VERSION, NOFUNC, NOFUNC, HLIB_LIST},
	title_info, 			/* title_info */
	long_info,				/* long_info */
	".FLI",                 /* default_suffi */
	MAXFRAMES,0,			/* max_write_frames, max_read_frames */
	flow_spec_best_fit, 	/* (*spec_best_fit)() */
	create_flow_file,		/* (*create_image_file)() */
	NOFUNC, 				/* (*open_image_file)() */
	close_flow_file,		/* (*close_image_file)() */
	NOFUNC, 				/* (*read_first_frame)() */
	NOFUNC, 				/* (*read_delta_next)() */
	flow_save_frames,		/* (*save_frames)() */
};

