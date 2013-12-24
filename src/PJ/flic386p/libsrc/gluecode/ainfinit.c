/*****************************************************************************
 * AINFINIT.C - Initialize an AnimInfo structure to default/reasonable values.
 ****************************************************************************/

#include "flicglue.h"

static char *modulename = __FILE__;

Errcode pj_animinfo_init(AnimInfo *ainfo)
/*****************************************************************************
 * the specified AnimInfo structure is initialized.
 * after calling this function, the client code MUST fill in the width and
 * height fields in the AnimInfo structure before passing it to flic creation.
 ****************************************************************************/
{
	 if (NULL == ainfo)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	pj_stuff_bytes(0, ainfo, sizeof(AnimInfo));

	ainfo->depth = 8;
	ainfo->millisec_per_frame = DEFAULT_AINFO_SPEED;
	ainfo->aspect_dx = 1;
	ainfo->aspect_dy = 1;

	return Success;
}
