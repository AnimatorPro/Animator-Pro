#include "jfile.h"
#include "fli.h"

Errcode pj_fli_seek_first(Flifile *flif)
/************************************************************************* 
 * Seeks to offset of first (Initial non-delta) frame in a Flifile in 
 * preparation for reading the first frame.
 *
 * Parameters:
 *		Flifile	*flif;		Flifile struct initialized with pj_fli_open.
 * Returns:
 *		Success (0) if all goes well, a negative error code if not.
 *		(see errcodes.h)
 *************************************************************************/
{
	return xffseek(flif->xf, flif->hdr.frame1_oset, XSEEK_SET);
}
