/*****************************************************************************
 * FLICINFO.C - Load AnimInfo with values from header of an already-open flic.
 ****************************************************************************/

#include "flicglue.h"

static char *modulename = __FILE__;

Errcode pj_flic_info(Flic *pflic, AnimInfo *pinf)
/*****************************************************************************
 * return information about an open flic.
 ****************************************************************************/
{
	Fli_head	*phdr;

	if (NULL == pflic || NULL == pinf)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	if (NULL == pflic->flifile)
		return Err_file_not_open;

	phdr = &pflic->flifile->hdr;

	pj_animinfo_init(pinf);

	pinf->width 	 = phdr->width;
	pinf->height	 = phdr->height;
	pinf->x 		 = 0;
	pinf->y 		 = 0;
	pinf->num_frames = phdr->frame_count;
	pinf->depth 	 = phdr->bits_a_pixel;
	pinf->aspect_dx  = phdr->aspect_dx;
	pinf->aspect_dy  = phdr->aspect_dy;

	pinf->millisec_per_frame = phdr->speed;

	return Success;
}
