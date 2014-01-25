/*****************************************************************************
 * TARGA.C - Main module for targa picture driver.
 ****************************************************************************/

#include "memory.h"
#include "targa.h"

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
	RL_KEYTEXT("targa_fmt")"Targa output format:\n"
	"Color Mapped\n"
	"Truecolor (RGB)\n"
	"Cancel\n"
	;

static char output_compression_options[] =
	RL_KEYTEXT("targa_comp")"Targa output compression:\n"
	"None\n"
	"Run Length\n"
	"Cancel\n"
	;

static Pdroptions save_options = {
	output_format_options,			/* option 1 qchoice selection list */
	output_compression_options, 	/* option 2 qchoice selection list */
	};

enum {
	OUTPUT_FORMAT_CMAPPED,			/* output format types, as returned  */
	OUTPUT_FORMAT_RGB,				/* from the Qchoice dialog			 */
	};

enum {
	OUTPUT_COMPRESS_NONE,			/* output compression schemes as	 */
	OUTPUT_COMPRESS_RLE,			/* returned from the Qchoice dialog  */
	};

#define OUTPUT_FORMAT_DEFAULT	OUTPUT_FORMAT_RGB			/* defaults  */
#define OUTPUT_COMPRESS_DEFAULT OUTPUT_COMPRESS_RLE

/*----------------------------------------------------------------------------
 * code...
 *--------------------------------------------------------------------------*/

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
	Targa_file *tf;

	if(ptf == NULL || *ptf == NULL)
		return;
	else
		tf = (Targa_file *)*ptf;

	if (tf->hsegbuf != NULL)
		pj_free(tf->hsegbuf);

	if (tf->lbuf != NULL)
		pj_free(tf->lbuf);

	if(tf->file != NULL)
		fclose(tf->file);

	pj_free(tf);
	*ptf = NULL;
	return;
}

static Errcode alloc_and_open(Targa_file **ptf, char *path, char *openmode)
/*****************************************************************************
 * allocate main data structure, open file.
 ****************************************************************************/
{
	Targa_file	 *tf;

	if (NULL == (tf = pj_zalloc(sizeof(Targa_file))))
		return Err_no_memory;
	*ptf = tf;

	if (NULL == (tf->file = fopen(path, openmode)))
		return pj_errno_errcode();

	return Success;
}

static Errcode open_file(Pdr *pd, char *path, Image_file **pif, Anim_info *ainfo )
/*****************************************************************************
 * PDR file open function for Targa files.
 ****************************************************************************/
{
	Errcode 	err;
	Targa_file	*tf;

	*pif = NULL;	/* in case of error */

	/*
	 * allocate main data structures, open file.
	 */

	if (Success != (err = alloc_and_open(&tf, path, "rb")))
		goto ERROR_EXIT;

	if (Success != (err = read_targa_header(tf)))
		goto ERROR_EXIT;

	if (ainfo != NULL)
		{
		ainfo->width  = tf->width;
		ainfo->height = tf->height;
		ainfo->depth  = tf->pdepth;
		ainfo->num_frames = 1;
		ainfo->millisec_per_frame = DEFAULT_AINFO_SPEED;
		}

	*pif = (Image_file *)tf;
	return Success;

ERROR_EXIT:

	close_file((Image_file **)&tf);
	*pif = NULL;

	return(err);
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
	Targa_file	*tf;

	tf	 = NULL;
	*pif = NULL;

	/*
	 * allocate main data structures, open file.
	 */

	if (Success != (err = alloc_and_open(&tf, path, "wb")))
		goto ERROR_EXIT;

	tf->width  = ainfo->width;
	tf->height = ainfo->height;

	if (NULL == (tf->hsegbuf = pj_malloc(tf->width)))
		{
		err = Err_no_memory;
		goto ERROR_EXIT;
		}

	*pif = (Image_file *)tf;
	return Success;

ERROR_EXIT:

	close_file((Image_file **)&tf);
	return err;

}

static Errcode read_first(Image_file *ifile, Rcel *screen)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Errcode err;
	Targa_file	*tf = (Targa_file *)ifile;

	if (tf->pdepth > 8)
		return Err_rgb_convert;

	tf->screen_rcel = screen;

	err = read_cmapped_image(tf);

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
	Targa_file	*tf = (Targa_file *)ifile;

	if (save_options.options_valid == FALSE)
		{
		save_options.option1 = OUTPUT_FORMAT_DEFAULT;
		save_options.option2 = OUTPUT_COMPRESS_DEFAULT;
		}

	switch (save_options.option1)
		{
		case OUTPUT_FORMAT_CMAPPED:
			tf->is_rgb = FALSE;
			tf->pdepth = 8;
			break;
		case OUTPUT_FORMAT_RGB:
			tf->is_rgb = TRUE;
			tf->pdepth = 24;
			break;
		default:
			return Err_driver_protocol;
		}

	switch (save_options.option2)
		{
		case OUTPUT_COMPRESS_NONE:
			tf->is_compressed = FALSE;
			break;
		case OUTPUT_COMPRESS_RLE:
			tf->is_compressed = TRUE;
			break;
		default:
			return Err_driver_protocol;
		}

	tf->screen_rcel = screen;

	if (Success != (err = write_targa_header(tf)))
		return err;

	err = write_targa_image(tf);

	return err;
}

/*****************************************************************************
 * Rexlib interface...
 ****************************************************************************/

static char targa_pdr_name[] = "TARGA.PDR";
static char title_info[] = "TARGA Image File Format.";

static char long_info[]  = RL_KEYTEXT("targa_info")
						   "Reads most TARGA image formats.  "
						   "Writes 24-bit RGB TARGA format.  "
						   "\n\n"
						   "To read truecolor TARGA files, or write files "
						   "with different color or compression schemes, "
						   "use the ANICONV program.  "
						   ;

static Pdr targa_pdr_header = {
	{ REX_PICDRIVER, PDR_VERSION, NOFUNC, NOFUNC, NULL, NULL, NULL },
	title_info, 		/* title_info */
	long_info,			/* long_info */
	".TGA;.PIX",             /* default_suffix */
	1,1,				/* max_write_frames, max_read_frames */
	spec_best_fit,		/* (*spec_best_fit)() */
	create_file,		/* (*create_file)() */
	open_file,			/* (*open_file)() */
	close_file, 		/* (*close_file)() */
	read_first, 		/* (*read_first_frame)() */
	NOFUNC, 			/* (*read_delta_next)() */
	save_frames,		/* (*save_frames)() */
	&save_options,		/* Pdroptions pointer */
	read_seekstart, 	/* (*rgb_seekstart)() */
	read_nextline,		/* (*rgb_readline()() */
};

Local_pdr targa_local_pdr = {
	NULL,
	targa_pdr_name,
	&targa_pdr_header
};
