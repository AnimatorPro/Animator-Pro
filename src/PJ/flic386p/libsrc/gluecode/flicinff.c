/*****************************************************************************
 * FLICINFF.C - Get info from a flic file, leaving file closed upon return.
 ****************************************************************************/

#include "flicglue.h"

Errcode pj_flic_file_info(char *path, AnimInfo *pinf)
/*****************************************************************************
 * open a flic file, get info from its header, close it, and return the info.
 * (note that parm checking is done by routine(s) we call).
 ****************************************************************************/
{
	Errcode 	err;
	Flic		flic;

	if (Success > (err = pj_flic_open_info(path, &flic, pinf))) {
		return err;
	}
	pj_flic_close(&flic);
	return Success;
}
