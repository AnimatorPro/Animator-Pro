/*****************************************************************************
 * FLICWRTN - Write the second and subsequent frames of a flic.
 ****************************************************************************/

#include "flicglue.h"

static char *modulename = __FILE__;

Errcode pj_flic_write_next(Flic *pflic,
						   FlicRaster *thisframe,
						   FlicRaster *priorframe)
/*****************************************************************************
 * generate delta packets between priorframe & thisframe, write them to disk.
 ****************************************************************************/
{
	Errcode 	err;
	Fli_frame	*cbuf = NULL;

	if (NULL == thisframe || NULL == priorframe ||
		NULL == pflic || NULL == pflic->flifile)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	if (pflic->flifile->hdr.frame_count == 0)
		return Err_uninit;

	if (Success <= (err = pj_fli_cel_alloc_cbuf(&cbuf, (Rcel *)thisframe)))
		err = pj_fli_add_next(NULL, pflic->flifile, cbuf,
					(Rcel *)priorframe, (Rcel *)thisframe);

	pj_freez(&cbuf);

	return err;

}
