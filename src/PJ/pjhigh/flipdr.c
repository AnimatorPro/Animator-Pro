/* this is a local pdr s that are set up for linking in to the code
 * and used by the pdr managing routines */

#include "animinfo.h"
#include "errcodes.h"
#include "fli.h"
#include "picdrive.h"
#include "pjbasics.h"

typedef struct fliif {
	Image_file ifile;
	Flifile ff;
} Fliif;


static Errcode fliif_read_first(Image_file *imf, Rcel *screen)
{
Fliif *flif = (Fliif *)imf;

	return(pj_fli_read_first(NULL,&flif->ff,screen,TRUE));  
}


static void close_fliif(Image_file **pif)
{
Fliif *flif;

	if((flif = (Fliif *)(*pif)) == NULL)
		return;
	pj_fli_close(&(flif->ff));
	pj_freez(pif);
}

static Errcode open_fliif(Pdr *pd, char *path, Image_file **pif, 
						  Anim_info *ainfo )
{
Fliif *flif;
Errcode err;
(void)pd;

	if(NULL == (*pif = pj_zalloc(sizeof(Fliif))))
		return(Err_no_memory);
	flif = (Fliif *)(*pif);

	if((err = pj_fli_info_open(&flif->ff, path, ainfo)) < Success)
		goto error;

	return(Success);
error:
	close_fliif(pif);
	return(err);
}

static Boolean fliif_spec_best_fit(Anim_info *ainfo)
{
	if(ainfo->depth == 8
		 && ainfo->num_frames == 1)
	{
		return(TRUE);
	}
	ainfo->depth = 8;
	ainfo->num_frames = 1;
	return(FALSE);
}
static Errcode create_fliif(Pdr *pd, char *path, Image_file **pif, 
						    Anim_info *ainfo )
{
Fliif *flif;
Errcode err;
(void)pd;

	if(NULL == (*pif = pj_zalloc(sizeof(Fliif))))
		return(Err_no_memory);
	flif = (Fliif *)(*pif);
	if((err = pj_fli_create(path,&flif->ff)) < Success)
		goto error;
	flif->ff.hdr.aspect_dx = ainfo->aspect_dx;
	flif->ff.hdr.aspect_dy = ainfo->aspect_dy;
	flif->ff.hdr.speed = ainfo->millisec_per_frame;
	flif->ff.hdr.width = ainfo->width;
	flif->ff.hdr.height = ainfo->height;
	goto done;
error:
	close_fliif(pif);
done:
	return(err);
}
static Errcode fliif_save_frames(Image_file *ifile, 
						   		 Rcel *screen, 
						   		 int num_frames, 
						   		 Errcode (*seek_frame)(int ix,void *seek_data),
						   		 void *seek_data,
						   		 Rcel *work_screen )
{
Fliif *flif = ((Fliif *)ifile);
(void)seek_frame;
(void)seek_data;
(void)work_screen;

	if(num_frames != 1)
		return(Err_bad_input);
	return(pj_write_one_frame_fli(NULL,&flif->ff,screen));
}

static char title_info[] =  "\"FLC\" Animator image file.";

static Pdr fli_pdr_head = {
	{ REX_PICDRIVER, PDR_VERSION, NOFUNC, NOFUNC, NULL },
	title_info,  			/* title_info */
	NULL,  					/* long_info */
	".FLC",			 		/* default_suffi */
	MAXFRAMES,MAXFRAMES,	/* max_write_frames, max_read_frames */
	fliif_spec_best_fit,	/* (*spec_best_fit)() */
	create_fliif,			/* (*create_image_file)() */
	open_fliif,				/* (*open_image_file)() */
	close_fliif,			/* (*close_image_file)() */
	fliif_read_first,		/* (*read_first_frame)() */
	NOFUNC,					/* (*read_delta_next)() */
	fliif_save_frames,	 	/* (*save_frames)() */
};

/* global items for outside */


char fli_pdr_name[] = "=FLC.PDR";

Local_pdr fli_local_pdr = {
	NULL,
	fli_pdr_name,
	&fli_pdr_head,
};

