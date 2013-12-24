/*****************************************************************************
 * FLICCLOS.C - Close a flic file opened for either input or output.
 ****************************************************************************/

#include "flicglue.h"

static char *modulename = __FILE__;

Errcode pj_flic_close(Flic *pflic)
/*****************************************************************************
 * close a flic and free all attached resources.
 ****************************************************************************/
{

	if (NULL == pflic)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);
	if (NULL != pflic->flifile)
		pj_fli_close(pflic->flifile);
	pj_freez(&pflic->flifile);
	pj_freez(&pflic->libctl);
	return Success;
}
