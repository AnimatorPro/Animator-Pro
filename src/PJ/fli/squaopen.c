#include "fli.h"

Errcode squawk_open_flifile(char *path, Flifile *flif, int jmode)

/* open flifile reporting errors */
{
Errcode err;

	if((err = pj_fli_open(path,flif,jmode)) < Success)
		err = pj_fli_error_report(err, "Can't open FLC \"%s\"", path );
	return(err);
}
