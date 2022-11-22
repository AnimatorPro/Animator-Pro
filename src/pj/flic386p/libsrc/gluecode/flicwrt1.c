/*****************************************************************************
 * FLICWRT1.C - Write the first frame of an output flic.
 ****************************************************************************/

#include "flicglue.h"

static char *modulename = __FILE__;

Errcode pj_flic_write_first(Flic *pflic, FlicRaster *firstframe)
/*****************************************************************************
 * write the first frame.
 * note that if multiple frames have already been written, calling this
 * function essentially throws them away, and starts the flic file over at
 * frame 1.  (I'm not totally sure it's safe to use this as a feature, tho!)
 ****************************************************************************/
{
	Errcode 	err;
	Fli_frame	*cbuf = NULL;

	if (NULL == firstframe || NULL == pflic || NULL == pflic->flifile)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	if (Success <= (err = pj_fli_cel_alloc_cbuf(&cbuf, (Rcel *)firstframe)))
		err = pj_fli_add_frame1(NULL, pflic->flifile, cbuf, (Rcel *)firstframe);

	pj_freez(&cbuf);

	return err;

}
