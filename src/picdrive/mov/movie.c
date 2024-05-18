/***************************************************************
Autodesk Movie file pdr modules:

	Created by Peter Kennard.  Oct 5, 1991
		These modules implement a PDR for Autodesk movie file compressed
		EGA pixel animations.  Slide records, buttons looping and other
		functions of movie files are not implemented.
****************************************************************/
#include "movie.h"

static char fhdr10[] = FHDR1;  /* id string for movie files */


static void cleanup_headers(Mfile *mf)
/* Free header index buffers */
{
	freez(&mf->hframe);
}
static Errcode read_alloc_headers(Mfile *mf, int num_frames)
/* Allocate buffers for header index tables and read in the data from the 
 * file. */
{
LONG bsize;

	cleanup_headers(mf);

	bsize = (sizeof(Framei) + sizeof(LONG) + sizeof(LONG) +sizeof(SHORT))
			* num_frames;

	if((mf->hframe = malloc(bsize)) == NULL)
		return(mf->lasterr = Err_no_memory);

	mf->hframea = OPTR(mf->hframe,sizeof(Framei)*num_frames);
	mf->hfllim =  OPTR(mf->hframea,sizeof(LONG)*num_frames);
	mf->hflen =  OPTR(mf->hfllim,sizeof(SHORT)*num_frames);

	if(mf_read_oset(mf,mf->hframe,bsize,sizeof(Mhead)) < Success)
		goto error;

	return(Success);
error:
	return(mf->lasterr);
}
static void cleanup_buffers(Mfile *mf)
/* Free all dynamic work buffers associated with a movie file. */
{
	cleanup_headers(mf);
}

static void close_movie_file(Image_file **pif)
/* Close file if open and deallocate all buffers. */
{
Mfile *mf;

	if((mf = *((Mfile **)pif)) == NULL)
		return;
	if(mf->file)
		fclose(mf->file);
	cleanup_buffers(mf);
	free(mf);
	*pif = NULL;
}
static Errcode mfile_open_sub(Mfile **pmf, char *path, char *fmode)
/* Allocate a mfile and open the file handle in input fmode. */
{
Errcode err;

	if((*pmf = zalloc(sizeof(Mfile))) == NULL)
		return(Err_no_memory);

	if(((*pmf)->file = fopen(path, fmode)) == NULL)
		goto ioerror;

	return(Success);

ioerror:
	err = pj_errno_errcode();
	close_movie_file((Image_file **)pmf);
	return(err);
}

static Errcode movie_read_first_frame(Image_file *ifile, Rcel *screen)
/* Now that we know the movie file has a depth <= 8 bits draw it in the 
 * screen. */
{
Mfile *mf = (Mfile *)ifile;

	mf->screen = screen;
	mf->cur_frame = -1;
	return(draw_next_frame(mf));
}

static Errcode movie_read_next_frame(Image_file *ifile, Rcel *screen)
/* Now that we have read the first frame, or one after it. Read the next 
 * available frame in the file and draw it in the screen. */
{
Mfile *mf = (Mfile *)ifile;

	mf->screen = screen;
	return(draw_next_frame(mf));
}


static Errcode open_movie_file(Pdr *pd, char *path, Image_file **pif,
							 Anim_info *ainfo )
/* Open an existing movie file and query for header info. */
{
Errcode err;
Mfile *mf;
Mhead mh;
int frame;
USHORT ftype;

	if((err = mfile_open_sub((Mfile **)pif, path, "rb")) < Success)
		return(err);

	mf = *((Mfile **)pif);

	if((err = mf_read(mf,&mh,sizeof(mh))) < Success)
		goto error;

	if(strncmp(fhdr10,mh.hmfhdr,sizeof(mh.hmfhdr)) != 0)
		goto bad_magic;

	mf->hdr = mh.h;

	/* Allocate and read record index tables */
	if(read_alloc_headers(mf, mh.h.hframes) < Success)
		goto error;	   

	/* Count up visible bit image frames. */
	for(frame = 0;frame < mh.h.hframes;++frame)
	{
	    ftype = mf->hframe[frame].ftype;
	    if(ftype & (FTBNOTIMG | FTBTEXT|FTBSLIDE|FTBMOVIE))
			continue;

		++mf->ainfo.num_frames;
	}

	mf->ainfo.width = XSIZE;
	mf->ainfo.height = YSIZE;
	mf->ainfo.depth = 8;
	mf->ainfo.millisec_per_frame = DEFAULT_AINFO_SPEED;

	if(ainfo)
		*ainfo = mf->ainfo;

success:
	return(Success);

bad_magic:
	err = Err_bad_magic;
	goto error;
error:
	close_movie_file(pif);
	return(err);
}

#ifdef SLUFFED
Errcode movie_save_frame(Image_file *ifile, Rcel *screen, int num_frames,
							  Errcode (*seek_frame)(int ix,void *seek_data),
							  void *seek_data, Rcel *work_screen )
{
	return(Err_unimpl);
}
static Errcode create_movie_file(Pdr *pd, char *path, Image_file **pif,
								 Anim_info *ainfo )
/* create a new movie file and prepare it for writing */
{
Errcode err;

	if((err = mfile_open_sub((Mfile **)pif,path,"wb")) >= Success)
		(*(Mfile **)pif)->ainfo = *ainfo;

	return(Err_unimpl);
}
static Boolean movie_spec_best_fit(Anim_info *ainfo)
{
Boolean nofit;

	nofit = (ainfo->depth == 8
			 && ainfo->num_frames == 1);

	ainfo->depth = 8;
	ainfo->num_frames = 1;
	return(nofit);
}
#endif /* SLUFFED */

/**** driver header declaration ******/

#define HLIB_TYPE_1 AA_STDIOLIB
#define HLIB_TYPE_2 AA_GFXLIB
#define HLIB_TYPE_3 AA_SYSLIB
#include "hliblist.h"

static char movie_title_info[] = "Autodesk movie format.";

static char long_info[] = "This module will read EGA format Autodesk movie "
						  "files.  It replaces the first 8 colors of the "
						  "palette with those used by the image.  The "
						  "other colors will remain unchanged.";

Pdr rexlib_header = {
	{ REX_PICDRIVER, PDR_VERSION, NOFUNC, NOFUNC, HLIB_LIST },
	movie_title_info, 		/* title_info */
	long_info,              /* long_info */
	".MOV",                 /* default_suffi */
	0,4000,					/* max_write_frames, max_read_frames */
	NOFUNC,					/* (*spec_best_fit)() */
	NOFUNC,					/* (*create_image_file)() */
	open_movie_file,		/* (*open_image_file)() */
	close_movie_file, 		/* (*close_image_file)() */
	movie_read_first_frame,	/* (*read_first_frame)() */
	movie_read_next_frame,	/* (*read_delta_next)() */
	NOFUNC, 				/* (*save_frames)() */
	NULL,					/* pdr options */
	NOFUNC,					/* (*rgb_seekstart)() */
	NOFUNC,     			/* (*rgb_readline)() */
};

