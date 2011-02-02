/*****************************************************************************
 * skeleton.c - a module to clone as a starting point for a picture driver.
 ****************************************************************************/

/*----------------------------------------------------------------------------
 * for debugging, the boxf() function displays text using printf-style input,
 * and waits for a keyhit.
 *--------------------------------------------------------------------------*/

#if 1
  #define debug(a)	boxf a
#else
  #define debug(a)
#endif

/*----------------------------------------------------------------------------
 * include the usual header files...
 *--------------------------------------------------------------------------*/

#include "picdrive.h"   /* required header file */
#include "errcodes.h"   /* most PDR modules will need error codes info  */
#include "gfx.h"        /* need gfx to access the screen */
#include "stdio.h"      /* and stdio to access the file */
#include "syslib.h"     /* and syslib to alloc/free memory */

/*----------------------------------------------------------------------------
 * set up the host libraries we need...
 *--------------------------------------------------------------------------*/

#define HLIB_TYPE_1 AA_GFXLIB
#define HLIB_TYPE_2 AA_STDIOLIB
#define HLIB_TYPE_3 AA_SYSLIB

#include <hliblist.h>			/* autobuild host library list */

/*----------------------------------------------------------------------------
 * driver description strings.
 *--------------------------------------------------------------------------*/

char short_description[] = "Skeleton (dummy) picture driver";
char long_description[]  = "This driver doesn't actually process "
						   "any picture file format.\n\n"
						   "It just demonstrates the major features "
						   "of coding a picture driver, and can be "
						   "cloned as a starting point for a new driver."
						   ;

/*----------------------------------------------------------------------------
 * options data.
 *	In init'ing the Pdroptions structure below, we specify only the two
 *	strings representing the Qchoice() menus the host will display.  The
 *	compiler inits all the rest of the Pdroptions fields (and notably the
 *	options_valid) field to zero.  If the host conducts the Qchoice dialogs
 *	on our behalf, it will set options_valid to non-zero, and when our
 *	create_ifile() function gets called, we'll notice that and use the
 *	requested options instead of the inbuilt defaults.
 *--------------------------------------------------------------------------*/

char style_selections[] =	 "Select output format:\n"
							 "Old Style\n"
							 "New Style\n"
							 "New Improved Style\n"
							 "Cancel\n"
							 ;

char compress_selections[] = "Select compression method:\n"
							 "No compression\n"
							 "Method 1\n"
							 "Method X\n"
							 "Method Y\n"
							 "Method Z\n"
							 "Cancel\n"
							 ;

Pdroptions output_options = {
	style_selections,				/* option 1 qchoice selection list */
	compress_selections,			/* option 2 qchoice selection list */
	/* two more strings could appear here */
	};

#define OUTPUT_STYLE_DEFAULT	2	/* default style = New Improved Style */
#define OUTPUT_COMPRESS_DEFAULT 0	/* default compression = None		  */

/*----------------------------------------------------------------------------
 * Skel_file structure, our extension to host's Image_file.
 *--------------------------------------------------------------------------*/

typedef struct skel_file {
	Image_file	hdr;			 /* PJ Image_file, must be first in struct! */
	FILE		*file;
	int 		width;
	int 		height;
	int 		pdepth;
	} Skel_file;

/*----------------------------------------------------------------------------
 * code...
 *--------------------------------------------------------------------------*/

Boolean spec_best_fit(Anim_info *ainfo)
/*****************************************************************************
 * as long as it's one frame of 8 bits-per-pixel, we can do it as requested.
 * if the host has asked for anything else, force the data in the Anim_info
 * to match what we can do, and return FALSE to indicate we tweaked it.
 ****************************************************************************/
{
	debug(("In spec_best_fit()...\n"));

	if (ainfo->depth == 8 && ainfo->num_frames == 1)
		return TRUE;

	ainfo->depth = 8;
	ainfo->num_frames = 1;
	return FALSE;
}

void close_file(Image_file **psf)
/*****************************************************************************
 * Clean up resources used by picture driver.
 ****************************************************************************/
{
	Skel_file *sf;

	debug(("In close_file()...\n"));

	if(psf == NULL || *psf == NULL) /* these could be NULL if we get   */
		return; 					/* called by our own error cleanup.*/
	else
		sf = (Skel_file *)*psf; 	/* retrieve and recast to our type */

	if (sf->file != NULL)			/* if file is open, 			   */
		fclose(sf->file);			/* close it.					   */

	free(sf);						/* free our control structure	   */
	*psf = NULL;					/* indicate it's now free          */
	return;

}

Errcode alloc_and_open(Skel_file **psf, char *path, char *openmode)
/*****************************************************************************
 * allocate main data structure, open file.
 ****************************************************************************/
{
	Skel_file	*sf;

	/*------------------------------------------------------------------------
	 * obtain some storage for our control structure, punt on error.
	 *----------------------------------------------------------------------*/

	if (NULL == (sf = zalloc(sizeof(Skel_file))))
		return Err_no_memory;
	*psf = sf;

	/*------------------------------------------------------------------------
	 * open the file, if open fails, return error code, else return success.
	 *----------------------------------------------------------------------*/

/*	if (NULL == (sf->file = fopen(path, openmode))) */
/*		return pj_errno_errcode();					*/

	return Success;
}

Errcode open_file(Pdr		 *pd,
				  char		 *path,
				  Image_file **pif,
				  Anim_info  *ainfo)
/*****************************************************************************
 * Open up the file, verify file header.
 ****************************************************************************/
{
	Errcode 	err;
	Skel_file	*sf;

	debug(("In open_file()...\nFile=%s\n", path));

	sf	 = NULL;			/* preset these to NULL in case we take our 	*/
	*pif = NULL;			/* error exit path before everything's alloc'd. */

	/*------------------------------------------------------------------------
	 * allocate main data structure, open file.
	 *----------------------------------------------------------------------*/

	if (Success != (err = alloc_and_open(&sf, path, "rb")))
		goto ERROR_EXIT;

	/*------------------------------------------------------------------------
	 * at this point, analyze the contents of the file to make sure it
	 * contains data in the format you expected.  there is no guarantee that
	 * the file is the type you expect -- in fact, quite often it won't be.
	 * if you don't recognize the file format, return Err_pic_unknown.
	 *
	 * for the example, analyze the file by looking at its file name.  if
	 * the chars 'dmy' show up in the filename, we assume someone is trying
	 * to excercise the dummy driver (us) to watch the sequence of debug()
	 * calls.  if the filename string contains 'dmy' we pretend we found a
	 * file holding a 640x480x8bit picture, unless it contains 'dmyrgb',
	 * in which case we pretend we found a 640x3x24bit picture. (3 lines
	 * because we don't wanna make 480 calls to the debug() routine in
	 * rgb_readlines(), and make you hit a key 480 times).
	 *----------------------------------------------------------------------*/

/*	if (!verify_file_header(sf)) {	*/
/*		err = Err_pic_unknown;		*/
/*		goto ERROR_EXIT;			*/

	if (stristr(path, "dmyrgb")) {
		sf->width  = 640;
		sf->height = 3;
		sf->pdepth = 24;
	} else if (stristr(path, "dmy")) {
		sf->width  = 640;
		sf->height = 480;
		sf->pdepth = 8;
	} else {
		err = Err_pic_unknown;
		goto ERROR_EXIT;
	}

	/*------------------------------------------------------------------------
	 * fill in values we return to the host via pointers.
	 *----------------------------------------------------------------------*/

	memset(ainfo, 0, sizeof(*ainfo));
	ainfo->width  = sf->width;
	ainfo->height = sf->height;
	ainfo->depth  = sf->pdepth;
	ainfo->num_frames = 1;
	ainfo->millisec_per_frame = DEFAULT_AINFO_SPEED;

	*pif = (Image_file *)sf;
	return Success;

ERROR_EXIT:

	close_file((Image_file **)&sf);
	return err;
}

Errcode read_first(Image_file *ifile, Rcel *screen)
/*****************************************************************************
 * read in 1st (er, only) image.
 ****************************************************************************/
{
	Errcode err;
	Skel_file *sf = (Skel_file *)ifile;

	debug(("In read_first()...\n"));

	if (sf->pdepth > 8) {				/* we don't do rgb via this routine */
		err = Err_rgb_convert;
		goto ERROR_EXIT;
	}

/*	pj_set_rast(screen, 0); 			*/		/* clear the screen */
/*	err = read_the_picture(sf, screen); */		/* load the picture */

ERROR_EXIT:

	return err;
}

Errcode read_next(Image_file *ifile, Rcel *screen)
/*****************************************************************************
 * Since we only have one frame this routine is pretty trivial.
 ****************************************************************************/
{
	debug(("In read_next()...\n"));
	return(Success);
}

Errcode rgb_seekstart(Image_file *pif)
/*****************************************************************************
 * this routine sets internal control information such that the next call
 * to rgb_readline() will obtain the first line of data for the image.
 *
 * the return value is 0 (Success) if image is rightsideup in the file,
 * and 1 if the image is upside down.  this routine may be called any number
 * of times during the process of reading an rgb file. it is generally called
 * twice, because it generally requires two passes for an rgb-aware host to
 * completely process the image data.  (first pass builds color map, second
 * pass loads data to screen while fitting it to the map.)
 ****************************************************************************/
{
	Skel_file *sf = (Skel_file *)pif;

	debug(("In rgb_seekstart()...\n"));

/*	fseek(sf->file, sf->data_offset, SEEK_SET); */ /* seek to data in file */

	return Success;
}

Errcode rgb_readline(Image_file *pif, Rgb3 *linebuf)
/*****************************************************************************
 * read the next line of rgb data and return status of read.
 *
 * generally speaking, the host will call this routine as many times as
 * there are lines in the file (based on ainfo->height set by the open_file()
 * routine).  It's best, of course, to check for EOF explicitly in your reader
 * and return Err_truncated if the host attempts to read too many lines.
 ****************************************************************************/
{
	Skel_file *sf = (Skel_file *)pif;

	debug(("In rgb_readline()...\n"));

/*	return read_some_rgb_data(sf, linebuf); */

	return Success;
}


Errcode create_file(Pdr 		 *pd,
					char		 *path,
					Image_file	 **pif,
					Anim_info	 *ainfo)
/*****************************************************************************
 * create an output file (alloc, open, and write file header).
 ****************************************************************************/
{
	Errcode 	err;
	Skel_file	*sf;

	debug(("In create_file()...\nFile=%s", path));

	sf	 = NULL;
	*pif = NULL;

	/*------------------------------------------------------------------------
	 * allocate main data structure, open file.
	 *----------------------------------------------------------------------*/

	if (Success != (err = alloc_and_open(&sf, path, "wb")))
		goto ERROR_EXIT;

	/*------------------------------------------------------------------------
	 * save what we need out of anim_info, and return success...
	 *----------------------------------------------------------------------*/

	 sf->width	= ainfo->width;
	 sf->height = ainfo->height;

	 *pif = (Image_file *)sf;
	 return Success;

ERROR_EXIT:

	close_file((Image_file **)&sf);
	return err;

}

Errcode save_frames(Image_file	 *ifile,
					Rcel		 *screen,
					int 		 num_frames,
					Errcode 	 (*seek_frame)(int ix,void *seek_data),
					void		 *seek_data,
					Rcel		 *work_screen )
/*****************************************************************************
 * save screen image.
 ****************************************************************************/
{
	Errcode 	err = Success;
	Skel_file	*sf = (Skel_file *)ifile;

	debug(("In save_frames()...\n"));

	/*------------------------------------------------------------------------
	 * if the host didn't conduct options dialogs, plug our defaults into
	 * the options structure.
	 *----------------------------------------------------------------------*/

	if (output_options.options_valid == FALSE) {
		output_options.option1 = OUTPUT_STYLE_DEFAULT;
		output_options.option2 = OUTPUT_COMPRESS_DEFAULT;
	}

	/*------------------------------------------------------------------------
	 * write the picture file from the input screen, using output options.
	 *----------------------------------------------------------------------*/

/*	  err = write_the_picture(sf, screen, output_options); */

	return err;
}


/*----------------------------------------------------------------------------
 * Setup rex & pdr interface structures...
 *--------------------------------------------------------------------------*/

Pdr rexlib_header = {
	{ REX_PICDRIVER, PDR_VERSION, NOFUNC, NOFUNC, HLIB_LIST },
	short_description,	/* title_info */
	long_description,	/* long_info */
	".DMY;.DM1;.DM?",   /* default_suffi */
	1,1,				/* max_write_frames, max_read_frames */
	spec_best_fit,		/* (*spec_best_fit)() */
	create_file,		/* (*create_image_file)() */
	open_file,			/* (*open_image_file)() */
	close_file, 		/* (*close_image_file)() */
	read_first, 		/* (*read_first_frame)() */
	read_next,			/* (*read_delta_next)() */
	save_frames,		/* (*save_frames)() */
	&output_options,	/* pointer to options structure */
	rgb_seekstart,		/* (*rgb_seekstart)() */
	rgb_readline,		/* (*rgb_readline() */
};


