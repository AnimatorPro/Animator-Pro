/*****************************************************************************
 * POPTINIT.C - Initialize a FlicPlayOptions structure to default values.
 *
 *	It is recommended that this be called against a FlicPlayOptions structure,
 *	then any values you want to supply options for can be overriden on a
 *	field-by-field basis after the call to this routine.  Following this
 *	procedure will keep your code compatible if new options are ever added
 *	to the structure in the future.  You will be able to recompile and link
 *	with a new version of the fliclib without worry about changing your
 *	code to account for new fields (especially if they have non-zero defaults).
 ****************************************************************************/

#include "flicglue.h"

static char *modulename = __FILE__;

Errcode pj_playoptions_init(FlicPlayOptions *poptions)
/*****************************************************************************
 * init a FlicPlayOptions structure to all default values.
 * note that defaults are zero/NULL for all items except speed, right now.
 ****************************************************************************/
{
	 if (NULL == poptions)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	pj_stuff_bytes(0, poptions, sizeof(FlicPlayOptions));

	poptions->speed = -1;

	return Success;

}
