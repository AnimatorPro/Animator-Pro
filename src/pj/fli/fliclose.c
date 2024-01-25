#include "jfile.h"
#include "fli.h"

void pj_fli_close(Flifile *flif)
/************************************************************************* 
 * Releases all resources associated with a Flifile.   This function
 * can be called more than once on the same Flifile, which is occassionally
 * useful when processing errors.
 * Parameters:
 *		Flifile *flif;		something initialized by pj_fli_open() or 
 *							pj_fli_create().
 *************************************************************************/
{
	xffclose(&flif->xf);
}
