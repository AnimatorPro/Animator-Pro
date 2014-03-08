#include "fli.h"

Errcode pj_fli_read_first(char *fname,        /* optional name for reporting 
										     * errors, will not report 
											 * if NULL */
						Flifile *flif, 
						struct rcel *fscreen,
						Boolean colors )  	/* update hw color map??? */

/* Seek to the first frame then call fli_read_next() to read it in 
 * from a FLI file and if not NULL, update the screen provided */
{
Errcode err;
	if((err = pj_fli_seek_first(flif)) < 0)
		return(err);
	return(pj_fli_read_next(fname,flif,fscreen,colors));
}
