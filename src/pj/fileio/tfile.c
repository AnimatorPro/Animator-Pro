/* tfile.c */

#include "tfile.h"
#include "filepath.h"
#include "pjassert.h"

/* Function: is_tdrive
 *
 *  Returns true if device is one of those
 *  non-alphabetic temporary or ram devices.
 */
Boolean is_tdrive(const char* device)
{
	if (!pj_assert(device != NULL))
		return FALSE;

	switch (device[0]) {
		case TDEV_MED:
		case TDEV_LO:
		case TRD_CHAR:
			return (device[1] == DEV_DELIM);

		default:
			return FALSE;
	}
}

/* Function: set_temp_path
 *
 *  Set new temp path.
 */
Errcode set_temp_path(const char* new_path)
{
	(void)new_path;
	return Success;
}

