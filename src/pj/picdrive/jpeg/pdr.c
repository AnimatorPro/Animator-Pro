#include "picdrive.h"
#include "errcodes.h"
#include "syslib.h"
#include "stdio.h"
#include "jinclude.h"
#include "jpegpdr.h"

typedef struct pdr_file {
/** This structure starts with and Image_file, and has further data
 ** local to the PDR.  
 **/
	Image_file hdr;
	FILE *file;
	Pixel *out_buf;
} Pdr_file;


static void pj_freez(void *pt)
{
	if(*((void **)pt) != NULL)
	{
		free(*((void **)pt));
		*((void **)pt) = NULL;
	}
}

static int to_upper(char a)
/* Convert lower case character to upper case.  Leave other characters
 * unchanged. */
{
if (a >= 'a' && a <= 'z')
	return(a + 'A' -  'a');
else
	return(a);
}

static txtcmp(char *a, char *b)
/* compare two strings ignoring case */
{
char aa,bb;

for (;;)
	{
	aa = to_upper(*a++);	/* fetch next characters converted to upper case */
	bb = to_upper(*b++);
	if (aa != bb)			/* if not equals return difference */
		return(aa-bb);
	if (aa == 0)
		return(0);
	}
}


suffix_in(string, suff)
char *string, *suff;
{
string += strlen(string) - strlen(suff);
return( txtcmp(string, suff) == 0);
}

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

static void close_image_file(Image_file **ptf)
/*****************************************************************************
 * Clean up resources used by picture driver.
 ****************************************************************************/
{
	Pdr_file *pf;

	if(ptf == NULL || *ptf == NULL)
		return;
	else
		pf = (Pdr_file *)*ptf;
	if (pf->file != NULL)
		fclose(pf->file);
	pj_freez(&pf->out_buf);
	pj_freez(ptf);
	return;
}


static Errcode create_image_file(Pdr			*pd,
						   char 		*path,
						   Image_file	**pif,
						   Anim_info	*ainfo)
/*****************************************************************************
 * create an output file (alloc, open, and write file header).
 ****************************************************************************/
{
	Errcode 	err;
	Pdr_file *pf = NULL;

	if ((pf = zalloc(sizeof(*pf))) == NULL)
		{
		err = Err_no_memory;
		goto ERROR;
		}
	if ((pf->out_buf = malloc((ainfo->width)*sizeof(Pixel))) == NULL)
		{
		err = Err_no_memory;
		goto ERROR;
		}
	if ((pf->file = fopen(path, "wb")) == NULL)
		{
		err = pj_errno_errcode();
		goto ERROR;
		}
	*pif = (Image_file *)pf;
	return Success;
ERROR:
	close_image_file((Image_file **)&pf);
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
	Pdr_file	*pf = (Pdr_file *)ifile;
	struct ifileref input;

	input.image = screen;
	input.line_buf = pf->out_buf;
	input.y = 0;

	return jpeg_save_frame(&input, pf->file);
}

static Errcode open_image_file(Pdr *pd, char *path
, Image_file **pif, Anim_info *ainfo )
/*****************************************************************************
 * PDR file open function for Targa files.
 ****************************************************************************/
{
	Errcode err;
	Pdr_file *pf = NULL;

	if (!suffix_in(path, ".JPG"))
		return(Err_suffix);

	if (ainfo != NULL)
		{
		int width, height;
		if ((err = jpeg_get_res(path, &width, &height)) < Success)
			goto ERROR;
		ainfo->width  = width;
		ainfo->height = height;
		ainfo->depth  = 8;
		ainfo->num_frames = 1;
		ainfo->millisec_per_frame = DEFAULT_AINFO_SPEED;
		}
	if ((pf = zalloc(sizeof(*pf))) == NULL)
		{
		err = Err_no_memory;
		goto ERROR;
		}
	if ((pf->file = fopen(path, "rb")) == NULL)
		{
		err = pj_errno_errcode();
		goto ERROR;
		}
	*pif = (Image_file *)pf;

	return Success;

ERROR:
	close_image_file((Image_file **)&pf);
	return err;
}

static Errcode read_first_frame(Image_file *ifile, Rcel *screen)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Pdr_file	*pf = (Pdr_file *)ifile;
	struct ifileref output;

	output.image = screen;
	output.line_buf = NULL;
	output.y = 0;

	return jpeg_read_frame(pf->file, &output);
}


/*****************************************************************************
 * REX/Pdr interface stuff...
 ****************************************************************************/

#define HLIB_TYPE_1 AA_STDIOLIB
#define HLIB_TYPE_2 AA_SYSLIB
#define HLIB_TYPE_3 AA_GFXLIB

#include "hliblist.h"

static char title_info[] = "JPEG Joint Photo Experts Group";

static char long_info[] = RL_KEYTEXT("jpeg_info")"Joint Photo Experts "
 "Group 'jfif' format.  This software is based in part on the work of the "
 "Independent JPEG Group.";

Pdr rexlib_header = {
	{ REX_PICDRIVER, PDR_VERSION, NOFUNC, NOFUNC, HLIB_LIST},
	title_info, 			/* title_info */
	long_info,				/* long_info */
	".JPG",                 /* default_suffi */
	1,1,					/* max_write_frames, max_read_frames */
	spec_best_fit, 			/* (*spec_best_fit)() */
	create_image_file,		/* (*create_image_file)() */
	open_image_file, 		/* (*open_image_file)() */
	close_image_file,		/* (*close_image_file)() */
	read_first_frame,		/* (*read_first_frame)() */
	NOFUNC, 				/* (*read_delta_next)() */
	save_frames,			/* (*save_frames)() */
};

