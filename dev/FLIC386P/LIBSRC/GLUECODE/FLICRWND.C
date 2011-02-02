/*****************************************************************************
 * FLICRWND.C - Rewind flic to first frame, don't display the frame.
 ****************************************************************************/

#include "flicglue.h"

static char *modulename = __FILE__;

Errcode pj_flic_rewind(Flic *pflic)
/*****************************************************************************
 * rewind the flic to the first frame, must be open for input.
 ****************************************************************************/
{
	if (NULL == pflic)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	if (JREADONLY != pflic->libctl->iotype)
		return pj_error_internal(Err_internal_parameter, modulename, __LINE__);

	pflic->libctl->cur_frame = 0;
	return Success;
}
