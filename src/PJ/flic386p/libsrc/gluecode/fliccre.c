/*****************************************************************************
 * FLICCRE.C - Create a new (or overwrite an existing) flic file.
 ****************************************************************************/

#include "flicglue.h"

static char *modulename = __FILE__;

Errcode pj_flic_create(char *path, Flic *pflic, AnimInfo *ainfo)
/*****************************************************************************
 * create/overwrite a flic file, prepare for output of frames.
 *
 * this routine allocates resources, opens the file, and writes the file
 * header.	(the file header gets rewritten at file close time, to update
 * the speed and num_frames values.)
 *
 * the values in the flic header are set baseed on values in the specified
 * AnimInfo structure.
 *
 * the output flic will be created in FLI-compatible format if the size is
 * 320x200.  for any other sizes, a FLC-format (variable rez) file is created.
 *
 * if the filename passed to this routine has no filetype on the end, a .FLI
 * or .FLC filetype (as appropriate) will be added.  if the filename already
 * includes a type (or even just ends in a '.', an empty type) it will not
 * be changed.
 ****************************************************************************/
{
	Errcode err;
	char	fullname[128];

	if (NULL == path || NULL == pflic || NULL == ainfo)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	if (0 >= ainfo->width || 0 >= ainfo->height)
		return pj_error_internal(Err_internal_parameter, modulename, __LINE__);

	strcpy(fullname, path);
	if (Success > (err = pj_flic_complete_filename(fullname, ainfo, FALSE)))
		return err;

	pflic->flifile = NULL;
	pflic->libctl  = NULL;

	if (NULL == (pflic->flifile = pj_zalloc(sizeof(Flifile))))
		return Err_no_memory;

	if (NULL == (pflic->libctl = pj_zalloc(sizeof(FliLibCtl)))) {
		pj_freez(&pflic->flifile);
		return Err_no_memory;
	}

	if (320 == ainfo->width && 200 == ainfo->height) {
		err = pj_fli_create_aa(fullname, pflic->flifile);
	} else {
		err = pj_fli_create(fullname, pflic->flifile);
	}

	if (Success > err) {
		pj_freez(&pflic->flifile);
		pj_freez(&pflic->libctl);
		return err;
	}

	pflic->flifile->hdr.width  = ainfo->width;
	pflic->flifile->hdr.height = ainfo->height;
	pflic->flifile->hdr.speed  = ainfo->millisec_per_frame;
	pflic->flifile->hdr.bits_a_pixel = ainfo->depth;
	pflic->flifile->hdr.frame_count  = 0;

	pflic->libctl->iotype = JREADWRITE;

	return Success;

}
