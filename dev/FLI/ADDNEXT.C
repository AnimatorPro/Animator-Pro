#include "fli.h"

Errcode pj_fli_add_next(char *name, /* name for error reporting 
								      * if NULL no reports */
					    Flifile *flif, void *cbuf,
					 	Rcel *last_screen, Rcel *this_screen)

/* calculates an intermediate delta record from the two rasters provided
 * and writes it to the output fli file.  Mus be called after a 
 * fli_add_frame1() or a previous fli_add_next() */
{
	pj_fli_comp_cel(cbuf, last_screen, this_screen, 
				 COMP_DELTA_FRAME, flif->comp_type);

	return(pj_i_add_next_rec(name,flif,cbuf));
}
