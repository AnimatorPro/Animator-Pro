/*****************************************************************************
 * FLICFINI.C - Finish a flic opened for output -- write the ring frame.
 ****************************************************************************/

#include "flicglue.h"

static char *modulename = __FILE__;

Errcode pj_flic_write_finish(Flic *pflic, FlicRaster *lastframe)
/*****************************************************************************
 * write the ring frame that loops a flic from the last to first frame.
 * this also causes the flic header to be flushed back to disk, which updates
 * the num_frames value in the header, and will also update the speed if it
 * has been changed with pj_flic_set_speed() since pj_flic_create() was called.
 ****************************************************************************/
{
	Errcode 	err;
	Fli_frame	*cbuf = NULL;
	FlicRaster	*workframe = NULL;

	if (NULL == lastframe || NULL == pflic || NULL == pflic->flifile)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	if (Success <= (err = pj_raster_make_ram(&workframe,
			lastframe->width, lastframe->height))) {
		if (Success <= (err = pj_fli_cel_alloc_cbuf(&cbuf, (Rcel *)lastframe))) {
			err = pj_fli_finish(NULL, pflic->flifile, cbuf,
					(Rcel *)lastframe, (Rcel *)workframe);
		}
	}

	pj_raster_free_ram(&workframe);
	pj_freez(&cbuf);

	return err;
}
