#include "fli.h"

Errcode pj_fli_add_frame1(char *name, /* name for error reporting 
								     * if NULL no reports */
					    Flifile *flif,
						void *cbuf,
						struct rcel *frame1)

/* will compress the first frame, from the screen provided, and call
 * fii_add_frame1_rec() with the resultant record */
{
	pj_fli_comp_cel(cbuf, NULL, frame1, COMP_FIRST_FRAME, flif->comp_type);
	return(pj_i_add_frame1_rec(name,flif,cbuf));
}
