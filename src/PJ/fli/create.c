#include "jfile.h"
#include "fli.h"

Errcode pj_fli_create(char *path, Flifile *flif)
/*************************************************************************
 * This routine is the first step in writing a fli.
 * Creates a new empty fli file and loads a Flifile for use with
 * other pj_fli_xxxx routines.
 *
 * Parameters:
 *		char	*path;			File name of fli.
 *		Flifile *flif;			Structure to control an open fli.
 * Returns:
 *		Success (0) if all goes well, a negative error code if not.
 *		(see errcodes.h)
 *************************************************************************/
{
	pj_stuff_bytes(0, flif, sizeof(*flif));
	flif->hdr.type = FLIHR_MAGIC;
	flif->comp_type = pj_fli_comp_ani;
	return(pj_i_create(path,flif));
}
