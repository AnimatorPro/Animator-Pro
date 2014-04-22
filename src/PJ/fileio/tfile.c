/* tfile.c */

#include "filepath.h"
#include "pjassert.h"
#include "tfile.h"

/* Function: is_tdrive
 *
 *  Returns true if device is one of those
 *  non-alphabetic temporary or ram devices.
 */
Boolean
is_tdrive(const char *device)
{
	if (!pj_assert(device != NULL)) return FALSE;

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
Errcode
set_temp_path(const char *new_path)
{
	(void)new_path;
	return Success;
}

/* Function: change_temp_path
 *
 *  set_temp_path and move old temp files into new_path.
 */
Errcode
change_temp_path(const char *new_path)
{
	return set_temp_path(new_path);
}

/* Function: trd_compact
 *
 *  Does nothing.  Originally used to move temporary files to disk.
 */
Errcode
trd_compact(long need_free)
{
	(void)need_free;
	return Success;
}
