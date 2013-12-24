#include "errcodes.h"
#include "jfile.h"
#include "fli.h"
#include "animinfo.h"

Errcode pj_fli_is_valid(char *path)

/* opens verifies validity of, and closes a fli file.
 * Returns Success if it is a valid fli file Errcode if not. */
{
	return(pj_fli_info(path,NULL));
}

