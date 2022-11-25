/*****************************************************************************
 * FLILO.C - Rex and PDR interface routines for low-rez FLI writer.
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "animinfo.h"
#include "cmap.h"
#include "errcodes.h"
#include "filepath.h"
#include "flilo.h"
#include "jfile.h"
#include "memory.h"
#include "picdrive.h"
#include "rastcomp.h"

static Errcode flow_i_flush_head(Flifile *flif)
/* Updates id and flushes header of a Flifile leaves file offset
 * at end of header */
{
	return xffwriteoset(flif->xf, &flif->hdr, 0, sizeof(flif->hdr));
}

static Errcode flow_i_add_next_rec(Flifile *flif, Fli_frame *frame )
{
	Errcode err;
	LONG size = ((Fli_frame *)frame)->size;

	err = xffwrite(flif->xf, frame, size);
	if (err < Success) {
		if (err == Err_eof)
			err = Err_truncated;

		/* attempt flush of header */
		flow_i_flush_head(flif);
		return err;
	}
	flif->hdr.size += size;
	++flif->hdr.frame_count;
	return Success;
}

static Errcode flow_add_frame1( Flifile *flif, void *cbuf, Rcel *frame1)
/* will compress the first frame, from the screen provided, and call
 * flow_i_add_next_rec() with the resultant record */

{
	flow_comp_cel(cbuf, NULL, frame1, FLI_BRUN);
	flif->hdr.size = xfftell(flif->xf);
	if (flif->hdr.size < 0)
		return (Errcode)flif->hdr.size;
	flif->hdr.frame_count = 0;
	return flow_i_add_next_rec(flif, cbuf);
}

static Errcode flow_add_next( Flifile *flif, void *cbuf,
							 Rcel *last_screen, Rcel *this_screen)

/* calculates an intermediate delta record from the two rasters provided
 * and writes it to the output fli file.  Must be called after a
 * flow_add_frame1() or a previous flow_add_next() */
{
	flow_comp_cel(cbuf, last_screen, this_screen, FLI_LC);
	return(flow_i_add_next_rec(flif,cbuf));
}

static Errcode flow_i_add_ring_rec(Flifile *flif, Fli_frame *frame )
{
Errcode err;
int ocount;

	ocount = flif->hdr.frame_count;
	if((err = flow_i_add_next_rec(flif,frame)) < Success)
		goto error;
	flif->hdr.frame_count = ocount;
	flif->hdr.flags = (FLI_FINISHED | FLI_LOOPED);
	err = flow_i_flush_head(flif);
error:
	return(err);
}

static Errcode flow_add_ring( Flifile *flif, void *cbuf,
							 Rcel *last_screen, Rcel *first_screen)
/* compresses record and writes a final ring frame (delta between last frame
 * and first) of a fli file sets header data and flushes the header. The file
 * is now complete and may be closed, or played. */
{
	flow_comp_cel(cbuf, last_screen, first_screen, FLI_LC);
	return(flow_i_add_ring_rec(flif,cbuf));
}

static Errcode flow_i_add_empty_ring(Flifile *flif)
/* Same as flow_add_ring() but writes a no change frame as the
 * next frame for blank flis */
{
Fli_frame frame;

	memset(&frame,0,sizeof(frame));
	frame.size = sizeof(frame);
	frame.type = FCID_FRAME;
	return(flow_i_add_ring_rec(flif,&frame));
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

static void close_flow_file(Image_file **pifile)
{
	Flifile *flif;

	flif = (Flifile *)(*pifile);
	if (flif == NULL)
		return;

	if (flif->xf != NULL)
		xffclose(&flif->xf);

	pj_free(flif);
	*pifile = NULL;
	--files_open;
}

static Errcode create_flow_file(Pdr *pd, char *path, Image_file **pif,
								 Anim_info *ainfo )
{
	Errcode err;
	Flifile *flif;
	(void)pd;

	if (files_open)
		return Err_too_many_files;

	*pif = pj_zalloc(sizeof(Flifile));
	if (*pif == NULL) {
		err = Err_no_memory;
		goto error;
	}

	flif = (Flifile *)(*pif);
	++files_open;

	err = xffopen(path, &flif->xf, XREADWRITE_CLOBBER);
	if (err < Success)
		goto error;

	flif->ifile.needs_work_cel = TRUE;
	flif->hdr.type = FLIH_MAGIC;
	flif->hdr.width = 320;
	flif->hdr.height = 200;
	flif->hdr.bits_a_pixel = 8;
	flif->hdr.speed = ((((long)ainfo->millisec_per_frame)*70L)+500L)/1000L;

	err = flow_i_flush_head(flif);
	if (err < Success)
		goto error;

	return Success;

error:
	close_flow_file(pif);
	return err;
}

static Errcode flow_save_frames(Image_file *ifile,
								Rcel *screen,
								ULONG num_frames,
								Errcode (*seek_frame)(int ix,void *seek_data),
								void *seek_data,
								Rcel *work_screen )
{
Errcode err = Success;
ULONG i;
Fli_frame *cbuf;
Flifile *flif = ((Flifile *)ifile);

	if(screen->width != 320 || screen->height != 200)
		return(Err_wrong_res);

	cbuf = NULL;
	for(i = 0;;++i)
	{
		if(NULL == (cbuf = pj_malloc(CBUF_SIZE)))
		{
			err = Err_no_memory;
			break;
		}
		if(i == 0)
		{
			if((err = flow_add_frame1(flif, cbuf, screen)) < Success)
				break;

			/* if only one frame terminate file and exit */

			if(num_frames <= 1)
			{
				err = flow_i_add_empty_ring(flif);
				break;
			}
		}
		else if(i < num_frames)
		{
			if((err = seek_frame(i,seek_data)) < Success)
				break;
			if((err = flow_add_next(flif,cbuf,work_screen,screen)) < Success)
				break;
		}
		else
		{
			/* if larger than one frame file re-seek to first frame */
			if((err = seek_frame(0,seek_data)) < Success)
				break;
			err = flow_add_ring(flif,cbuf,work_screen,screen);
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

static char flilo_pdr_name[] = "FLILORES.PDR";
static char title_info[] = "Animator 1.0 320 X 200 FLI format.";

static char long_info[] = RL_KEYTEXT("flilo_info")"Will create files readable"
						  " by low resolution Animator.";

static Pdr flilo_pdr_header = {
	{ REX_PICDRIVER, PDR_VERSION, NOFUNC, NOFUNC, NULL, NULL, NULL },
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

Local_pdr flilo_local_pdr = {
	NULL,
	flilo_pdr_name,
	&flilo_pdr_header,
};
