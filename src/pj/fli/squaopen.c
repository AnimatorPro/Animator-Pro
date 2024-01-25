#include "fli.h"

/* Function: squawk_open_flifile
 *
 *  Open flifile reporting errors.
 */
Errcode
squawk_open_flifile(char *path, Flifile *flif, enum XReadWriteMode mode)
{
	Errcode err;

	err = pj_fli_open(path, flif, mode);
	if (err < Success)
		err = pj_fli_error_report(err, "Can't open FLC \"%s\"", path );

	return err;
}
