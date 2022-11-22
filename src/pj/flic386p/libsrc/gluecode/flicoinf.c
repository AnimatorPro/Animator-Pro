/*****************************************************************************
 * FLICOINF.C - Open a flic, get info from its header, leave flic file open.
 ****************************************************************************/

#include "flicglue.h"

static char *modulename = __FILE__;

Errcode pj_flic_open_info(char *path, Flic *pflic, AnimInfo *pinf)
/*****************************************************************************
 * open a flic, and return info about it as well as leaving it open.
 * (note that some parm checking is done by routine(s) we call).
 ****************************************************************************/
{
	Errcode 	err;

	if (NULL == pinf)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	if (Success > (err = pj_flic_open(path, pflic))) {
		return err;
	}

	return pj_flic_info(pflic, pinf);
}
