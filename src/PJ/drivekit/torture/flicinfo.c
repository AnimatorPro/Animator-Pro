#include "torture.h"
#include "fli.h"
#include "animinfo.h"

Errcode get_flic_info(char *filename, struct anim_info *pinf)
/*****************************************************************************
 * return information about an open flic.
 ****************************************************************************/
{
	Errcode 	err = Success;
	Fli_head	flichead;
	FILE		*f = NULL;

	if (NULL == (f = fopen(filename, "rb"))) {
		err = Err_nogood;
		goto ERROR_EXIT;
	}

	if (1 != fread(&flichead, sizeof(flichead), 1, f)) {
		err = Err_nogood;
		goto ERROR_EXIT;
	}

	if (flichead.type != FLIH_MAGIC && flichead.type != FLIHR_MAGIC) {
		err = Err_bad_magic;
		goto ERROR_EXIT;
	}

	pinf->width 	 = flichead.width;
	pinf->height	 = flichead.height;
	pinf->x 		 = 0;
	pinf->y 		 = 0;
	pinf->num_frames = flichead.frame_count;
	pinf->depth 	 = flichead.bits_a_pixel;
	pinf->aspect_dx  = flichead.aspect_dx;
	pinf->aspect_dy  = flichead.aspect_dy;

	pinf->millisec_per_frame = flichead.speed;

ERROR_EXIT:

	if (f != NULL)
		fclose(f);

	return err;
}
