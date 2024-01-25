/* flipdr.c - this makes up a pic-driver for the built-in FLC load/save
 * routines.
 */
#include "errcodes.h"		/* negative error codes */
#include "memory.h"
#include "picdrive.h"
#include "fli.h"			/* fli file stuff */
#include "filepath.h"

typedef struct ifile
	{
	Image_file hdr;
	Flifile flif;
	char name[PATH_SIZE];
	} Ifile;

static Boolean spec_best_fit(Anim_info *ainfo)
/*****************************************************************************
 * Tell host that we can only write 8 bit-a-pixel images,  and not
 * more than MAXFRAMES.
 ****************************************************************************/
{
	if(ainfo->depth == 8
		 && ainfo->num_frames <= MAXFRAMES)
	{
		return(TRUE);
	}

	ainfo->depth = 8;
	if(ainfo->num_frames > MAXFRAMES)
		ainfo->num_frames = MAXFRAMES;
	return(FALSE);
}

static Errcode create_file(Pdr *pd, char *path, Image_file **pif, 
							     Anim_info *ainfo )
/*****************************************************************************
 * Fire up a fli for writing.
 ****************************************************************************/
{
Errcode err;
Ifile *gf;

if (!spec_best_fit(ainfo))
	return(Err_pdepth_not_avail);
if(NULL == (*pif = pj_zalloc(sizeof(*gf))))
	return(Err_no_memory);
gf = (Ifile *)(*pif);
strcpy(gf->name, path);
if ((err = pj_fli_create(path, &gf->flif)) >= Success)
	{
	gf->flif.hdr.width = ainfo->width;
	gf->flif.hdr.height = ainfo->height;
	gf->flif.hdr.speed = ainfo->millisec_per_frame;
	gf->flif.hdr.frame_count = ainfo->num_frames;
	}
return(err);
}

static Errcode open_file(Pdr *pd, char *path, Image_file **pif, 
						 Anim_info *ainfo )
/*****************************************************************************
 * Psuedo PDR file open function for FLC files.
 ****************************************************************************/
{
Errcode err = Success;
Ifile *gf;

if (((*pif) = pj_zalloc(sizeof(*gf))) == NULL)
	return(Err_no_memory);
gf = (Ifile *)*pif;
strcpy(gf->name, path);
if ((err = pj_fli_open(path, &gf->flif, JREADONLY)) < Success)
	{
	close_file(pif);
	goto OUT;
	}
if (ainfo != NULL)
	{
	ainfo->width = gf->flif.hdr.width;
	ainfo->height = gf->flif.hdr.height;
	ainfo->num_frames = gf->flif.hdr.frame_count;
	ainfo->millisec_per_frame = gf->flif.hdr.speed;
	ainfo->depth = 8;
	}
OUT:
return(err);
}

static void close_file(Image_file **pif)
/*****************************************************************************
 * Close down the fli.
 ****************************************************************************/
{
Ifile *gf;
if ((gf = (Ifile *)*pif) != NULL)
	{
	pj_fli_close(&gf->flif);
	pj_freez(pif);
	}
}

static Errcode read_first(Image_file *ifile, Rcel *screen)
/*****************************************************************************
 * Read first frame of fli in PDR format.
 ****************************************************************************/
{
#define gf ((Ifile *)ifile)
return(pj_fli_read_first(gf->name, &gf->flif, screen, FALSE));
#undef gf
}

static Errcode read_next(Image_file *ifile, Rcel *screen)
/*****************************************************************************
 * Read subsequent frame of fli in PDR format.
 ****************************************************************************/
{
#define gf ((Ifile *)ifile)
return(pj_fli_read_next(gf->name, &gf->flif, screen, FALSE));
#undef gf
}

static Errcode save_frames(Image_file *ifile, Rcel *screen, int num_frames,
						      Errcode (*seek_frame)(int ix,void *seek_data),
						      void *seek_data, Rcel *work_screen ) 
/*****************************************************************************
 * PDR style save a FLIC 
 ****************************************************************************/
{
Errcode err;
Fli_frame *cbuf = NULL;
int i;
Ifile *gf = (Ifile *)ifile;
char *name = gf->name;

				/* Do the first frame */
if((err = pj_fli_cel_alloc_cbuf(&cbuf,screen)) < Success)
	goto error;
if((err = pj_fli_add_frame1(name, &gf->flif, 
							cbuf, screen)) < Success)
	goto error;
pj_freez(&cbuf);	/* We're allocating and freeing this buffer a lot to
					 * let the seek frame() routine use the memory if it
					 * needs it.... */
pj_rcel_copy(screen, work_screen);
for (i=1; i<num_frames; ++i)
	{
	if ((err = seek_frame(i, seek_data)) < Success)
		goto error;
	if((err = pj_fli_cel_alloc_cbuf(&cbuf,screen)) < Success)
		goto error;
	if((err = pj_fli_add_next(name,&gf->flif,cbuf,
							work_screen,screen)) < Success)
		goto error;
	pj_freez(&cbuf);
	pj_rcel_copy(screen, work_screen);
	}
if ((err = seek_frame(0, seek_data)) < Success)	/* go back for ring frame */
	goto error;
if((err = pj_fli_cel_alloc_cbuf(&cbuf,screen)) < Success)
	goto error;
if((err = pj_fli_add_ring(name,&gf->flif,cbuf,
					   work_screen,screen)) < Success)
	goto error;
error:
pj_freez(&cbuf);
return(err);
}

static char title_info[] =  "\"FLC\" Animator image file.";

static Pdr fli_pdr = {
	{ REX_PICDRIVER, PDR_VERSION, NOFUNC, NOFUNC, NULL },
	title_info,  			/* title_info */
	"",			  			/* long_info */
	".FLC;.FLI",	 		/* default_suffi */
	MAXFRAMES,MAXFRAMES,	/* max_write_frames, max_read_frames */
	spec_best_fit,			/* (*spec_best_fit)() */
	create_file,			/* (*create_image_file)() */
	open_file,				/* (*open_image_file)() */
	close_file,				/* (*close_image_file)() */
	read_first,				/* (*read_first_frame)() */
	read_next,				/* (*read_delta_next)() */
	save_frames,	    	/* (*save_frames)() */
};

char fli_pdr_name[] = "=fli.pdr";

Local_pdr fli_conv_pdr = {
	NULL,
	fli_pdr_name,
	&fli_pdr,
};

