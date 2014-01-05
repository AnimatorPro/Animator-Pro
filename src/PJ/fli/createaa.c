#include "fli.h"
#include "memory.h"

Errcode pj_fli_create_aa(char *path, Flifile *flif)

/* Creates a new empty fli file and load a Flifile for use with
 * Flifile routines. this will always leave file position at end of header.
 * Returns Success if OK. Closes, cleans up and returns Errcode if not */
{
	pj_stuff_bytes(0, flif, sizeof(*flif));
	flif->hdr.type = FLIH_MAGIC;
	flif->comp_type = pj_fli_comp_aa;
	return(pj_i_create(path,flif));
}
