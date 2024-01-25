// #define DEBUG_TOSCREEN
// #define DEBUG_TOFILE

/*****************************************************************************
 * TIFF.C - Main module for TIFF picture driver.  This driver reads tiff
 *			files of all types, and writes colormapped files.
 ****************************************************************************/

#include "tiff.h"

/*----------------------------------------------------------------------------
 * Options data...
 *	This is used by the CONVERT host to conduct Qchoice() dialogs with the
 *	user before calling our create_image_file() function.  The save_options
 *	structure passes to the host the strings containing the Qchoice title and
 *	options.  It also contains fields (not seen below, but init'd to
 *	zero by their absence) in which the host will return the option values
 *	gathered during the Qchoice dialogs.  When the dialogs have occurred,
 *	'save_options.options_valid' will be TRUE, and we use the specified values
 *	(for which mnemonics appear in the enums below).  If 'options_valid' is
 *	false, we have been loaded by a host that doesn't support options dialogs,
 *	and we will use defaults.
 *--------------------------------------------------------------------------*/

static char output_format_options[] = /* note that these are one big string */
	RL_KEYTEXT("tiff_fmt")"TIFF output format:\n"
	"Greyscale\n"
	"Color Mapped\n"
	"Truecolor (RGB)\n"
	"Cancel\n"
	;

static char output_compression_options[] =
	RL_KEYTEXT("tiff_comp")"TIFF output compression:\n"
	"None\n"
	"Packbits\n"
	"LZW\n"
	"Cancel\n"
	;

static Pdroptions save_options = {
	output_format_options,			/* option 1 qchoice selection list */
	output_compression_options, 	/* option 2 qchoice selection list */
	};

enum {
	OUTPUT_FORMAT_GREY, 			/* output format types as returned	 */
	OUTPUT_FORMAT_CMAPPED,			/* from the Qchoice dialog			 */
	OUTPUT_FORMAT_RGB,
	};

static int photometric_type[] = {	/* table to map Qchoice format types */
	PHMET_GREY_0ISBLACK,			/* to TIFF tag values.				 */
	PHMET_PALETTE_COLOR,
	PHMET_RGB,
	};

enum {
	OUTPUT_COMPRESS_NONE,			/* output compression schemes as	 */
	OUTPUT_COMPRESS_PACKBITS,		/* returned from the Qchoice dialog  */
	OUTPUT_COMPRESS_LZW,
	};

static int compression_type[] = {	/* table to map Qchoice compression  */
	CMPRS_NONE, 					/* types to TIFF tag values.		 */
	CMPRS_PACKBITS,
	CMPRS_LZW,
	};

#if defined(CMAPPED)
#define OUTPUT_FORMAT_DEFAULT	OUTPUT_FORMAT_CMAPPED			/* defaults  */
#define OUTPUT_COMPRESS_DEFAULT OUTPUT_COMPRESS_LZW
static char title_info[] = "Tag Image File Format.";

static char long_info[]  = RL_KEYTEXT("tiff_cmapped_info")
   "Reads monochrome and color mapped formats.  " 
   "Writes TIFF image files in color mapped format "
   "using LZW compression."
   "\n\n"
   "To read RGB TIFF files, or write TIFF files "
   "using different color options or compression "
   "schemes, use the ANICONV program."
   ;

#elif defined(RGB)
#define OUTPUT_FORMAT_DEFAULT	OUTPUT_FORMAT_RGB			/* defaults  */
#define OUTPUT_COMPRESS_DEFAULT OUTPUT_COMPRESS_PACKBITS
static char title_info[] = "RGB Tag Image File Format.";
static char long_info[]  = RL_KEYTEXT("tiff_rgb_info")
	"Reads monochrome and color mapped formats.  " 
	"Writes TIFF image files in RGB format "
	"using packbits compression."
	"\n\n"
	"To read RGB TIFF files, or write TIFF files "
	"using different color options or compression "
	"schemes, use the ANICONV program."
	;
#elif defined(GREY)
#define OUTPUT_FORMAT_DEFAULT	OUTPUT_FORMAT_GREY			/* defaults  */
#define OUTPUT_COMPRESS_DEFAULT OUTPUT_COMPRESS_PACKBITS
static char title_info[] = "Greyscale Tag Image File Format.";
static char long_info[]  = RL_KEYTEXT("tiff_grey_info")
	"Reads monochrome and color mapped formats.  " 
	"Writes TIFF image files in greys only "
	"using packbits compression."
	"\n\n"
	"To read RGB TIFF files, or write TIFF files "
	"using different color options or compression "
	"schemes, use the ANICONV program."
	;
#else
#error "Must define RGB, GREY, or CMAPPED"
#endif

/*----------------------------------------------------------------------------
 * ...end of options data.
 *--------------------------------------------------------------------------*/

/*****************************************************************************
 * some debugging junk, included only if debug symbols are defined above.
 ****************************************************************************/

#if defined(DEBUG_TOFILE)
  FILE *dbgfile;
  void debug_fopen()
  {
	dbgfile = fopen("tiffdbg.txt","w");
  }
  void debug_fclose()
  {
	fclose(dbgfile);
  }
  void debug_output(char *str)
  {
	fputs(str,dbgfile);
  }
#elif defined(DEBUG_TOSCREEN)
  void debug_output(char *string)
  {
	  while (*string)
		  dos_put_char(*string++);
  }
#endif

static Boolean spec_best_fit(Anim_info *ainfo)
/*****************************************************************************
 * as long as it's one frame of 8 bits-per-pixel, we can do it.
 ****************************************************************************/
{
	if (ainfo->depth == 8 && ainfo->num_frames == 1)
		return TRUE;

	ainfo->depth = 8;
	ainfo->num_frames = 1;
	return FALSE;
}

static void close_file(Image_file **ptf)
/*****************************************************************************
 * Clean up resources used by picture driver.
 ****************************************************************************/
{
	Tiff_file *tf;

	if(ptf == NULL || *ptf == NULL)
		return;
	else
		tf = (Tiff_file *)*ptf;

	if(tf->file != NULL)
		fclose(tf->file);

	if (tf->strip_data != NULL)
		free(tf->strip_data);

	if (tf->color_table != NULL)
		free(tf->color_table);

	if (tf->stripbuf != NULL)
		free(tf->stripbuf);

	if (tf->lzwbuf != NULL)
		free(tf->lzwbuf);

	if (tf->unlzwtable != NULL)
		free(tf->unlzwtable);

	if (tf->rbuf != NULL)
		free(tf->rbuf);

	free(tf);
	*ptf = NULL;
	return;
}

static Errcode alloc_and_open(Tiff_file **ptf, char *path, char *openmode)
/*****************************************************************************
 * allocate main data structure, open file.
 ****************************************************************************/
{
	Tiff_file	*tf;

	if (NULL == (tf = zalloc(sizeof(Tiff_file))))
		return Err_no_memory;
	*ptf = tf;

	if (NULL == (tf->file = fopen(path, openmode)))
		return pj_errno_errcode();

	return Success;
}

static Errcode open_file(Pdr		*pd,
						 char		*path,
						 Image_file **pif,
						 Anim_info	*ainfo)
/*****************************************************************************
 * Open up the file, verify file header.
 ****************************************************************************/
{
	Errcode 	err;
	Tiff_file	*tf;

	tf	 = NULL;
	*pif = NULL;

	/*------------------------------------------------------------------------
	 * allocate main data structure, open file.
	 * read file header and tags, validates structure of file.
	 *----------------------------------------------------------------------*/

	if (Success != (err = alloc_and_open(&tf, path, "rb")))
		goto ERROR_EXIT;

	if (Success != (err = read_filehdr(tf)))
		goto ERROR_EXIT;

	if (Success != (err = read_tiftags(tf)))
		goto ERROR_EXIT;

	/*------------------------------------------------------------------------
	 * check for things we can't handle yet.
	 *----------------------------------------------------------------------*/

	switch (tf->compression)
		{
		case CMPRS_NONE:
		case CMPRS_WNONE:
		case CMPRS_1DHUFFMAN:
		case CMPRS_PACKBITS:
		case CMPRS_LZW:
			break;
		default:
			err = Err_wrong_type;
			goto ERROR_EXIT;
		}

	switch (tf->photometric)
		{
		case 0:
		case 1:
		case 2:
		case 3:
			break;
		default:
			err = Err_wrong_type;
			goto ERROR_EXIT;
		}

	/*------------------------------------------------------------------------
	 * we can do it!  fill out PJ's return-value structures.
	 *----------------------------------------------------------------------*/

	if (ainfo != NULL)		/* would this ever be NULL??? */
		{
		memset(ainfo, 0, sizeof(*ainfo));
		ainfo->width  = tf->width;
		ainfo->height = tf->height;
		ainfo->depth  = tf->pixel_depth;
		ainfo->num_frames = 1;
		ainfo->millisec_per_frame = DEFAULT_AINFO_SPEED;
		}

	*pif = (Image_file *)tf;
	return Success;

ERROR_EXIT:

	close_file((Image_file **)&tf);
	return err;
}


static Errcode read_first(Image_file *ifile, Rcel *screen)
/*****************************************************************************
 * read in 1st (er, only) image.
 ****************************************************************************/
{
	Errcode err;
	Tiff_file *tf = (Tiff_file *)ifile;

	if (tf->samples_per_pixel != 1) /* we don't do rgb via this routine */
		{
		err = Err_rgb_convert;
		goto ERROR_EXIT;
		}

	tf->screen_rcel = screen;
	pj_set_rast(screen, 0);

	err = toscreen_monoplane_image(tf);

ERROR_EXIT:

	return err;
}

static Errcode read_next(Image_file *ifile, Rcel *screen)
/*****************************************************************************
 * Since we only have one frame this routine is pretty trivial.
 ****************************************************************************/
{
	return(Success);
}

static Errcode create_file(Pdr			*pd,
						   char 		*path,
						   Image_file	**pif,
						   Anim_info	*ainfo)
/*****************************************************************************
 * create an output file (alloc, open, and write file header).
 ****************************************************************************/
{
	Errcode 	err;
	Tiff_file	*tf;

	tf	 = NULL;
	*pif = NULL;

	/*
	 * allocate main data structure, open file.
	 */

	if (Success != (err = alloc_and_open(&tf, path, "wb")))
		goto ERROR_EXIT;

	/*
	 * save what we need out of anim_info, and return success...
	 */

	 tf->width	= ainfo->width;
	 tf->height = ainfo->height;

	 *pif = (Image_file *)tf;
	 return Success;

ERROR_EXIT:

	close_file((Image_file **)&tf);
	return err;

}

static Errcode save_frames(Image_file	*ifile,
						   Rcel 		*screen,
						   int			num_frames,
						   Errcode		(*seek_frame)(int ix,void *seek_data),
						   void 		*seek_data,
						   Rcel 		*work_screen )
/*****************************************************************************
 * save screen image.
 ****************************************************************************/
{
	Errcode 	err;
	int 		photometric;
	int 		compression;
	Tiff_file	*tf = (Tiff_file *)ifile;

	tf->screen_rcel = screen;

	if (save_options.options_valid == FALSE)
		{
		save_options.option1 = OUTPUT_FORMAT_DEFAULT;
		save_options.option2 = OUTPUT_COMPRESS_DEFAULT;
		}

	photometric = photometric_type[save_options.option1];
	compression = compression_type[save_options.option2];

	if (Success != (err = write_filehdr(tf)))
		return err;

	err = fromscreen_monoplane_image(tf, photometric, compression);

	return err;
}

/*****************************************************************************
 * Rexlib interface...
 ****************************************************************************/

#define HLIB_TYPE_1 AA_SYSLIB
#define HLIB_TYPE_2 AA_STDIOLIB
#define HLIB_TYPE_3 AA_GFXLIB
#include "hliblist.h"

#if defined(DEBUG_TOFILE)
  #define TIFFINIT	((void*)debug_fopen)
  #define TIFFCLEAN ((void*)debug_fclose)
#else
  #define TIFFINIT	NOFUNC
  #define TIFFCLEAN NOFUNC
#endif

Pdr rexlib_header = {
	{ REX_PICDRIVER, PDR_VERSION, TIFFINIT, TIFFCLEAN, HLIB_LIST },
	title_info, 		/* title_info */
	long_info,			/* long_info */
	".TIF",             /* default_suffix */
	1,1,				/* max_write_frames, max_read_frames */
	spec_best_fit,		/* (*spec_best_fit)() */
	create_file,		/* (*create_image_file)() */
	open_file,			/* (*open_image_file)() */
	close_file, 		/* (*close_image_file)() */
	read_first, 		/* (*read_first_frame)() */
	read_next,			/* (*read_delta_next)() */
	save_frames,		/* (*save_frames)() */
	&save_options,		/* pointer to options structure */
	rgb_seekstart,		/* (*rgb_seekstart)() */
	rgb_readline,		/* (*rgb_readline() */
};
