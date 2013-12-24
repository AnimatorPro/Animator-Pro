/*****************************************************************************
 * FLICOPEN.C - Open a flic file for input.
 ****************************************************************************/

#include "flicglue.h"

static char *modulename = __FILE__;

Errcode pj_flic_open(char *path, Flic *pflic)
/*****************************************************************************
 * alloc flic control structures, open flic file, return status.
 ****************************************************************************/
{
	Errcode err;

	if (NULL == pflic || NULL == path)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	if ('\0' == *path)
		return pj_error_internal(Err_internal_parameter, modulename, __LINE__);

	pflic->flifile = NULL;
	pflic->libctl  = NULL;

	if (NULL == (pflic->flifile = pj_zalloc(sizeof(Flifile))))
		return Err_no_memory;

	if (NULL == (pflic->libctl = pj_zalloc(sizeof(FliLibCtl)))) {
		pj_freez(&pflic->flifile);
		return Err_no_memory;
	}

	if (Success > (err = pj_fli_open(path, pflic->flifile, JREADONLY))) {
		pj_freez(&pflic->flifile);
		pj_freez(&pflic->libctl);
		return err;
	}

	pflic->libctl->iotype = JREADONLY;
	pflic->libctl->cur_frame = 0;

	return Success;
}

