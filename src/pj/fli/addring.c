#include "fli.h"

Errcode pj_fli_add_ring(char *name, /* name for error reporting 
								     * if NULL no reports */
					    Flifile *flif, void *cbuf,
						struct rcel *last_screen, struct rcel *first_screen)

/* compresses record and writes a final ring frame (delta between last frame 
 * and first) of a fli file sets header data and flushes the header. The file
 * is now complete and may be closed, or played. */
{
	pj_fli_comp_cel(cbuf, last_screen, first_screen, COMP_DELTA_FRAME,flif->comp_type);
	return(pj_i_add_ring_rec(name,flif,cbuf));
}
