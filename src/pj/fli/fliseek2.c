#ifdef SLUFFED
#include "jfile.h"
#include "fli.h"

Errcode pj_fli_seek_second(Flifile *flif)
/************************************************************************* 
 * Seeks to offset of second (delta from first to second) frame in a Flifile.
 * Typically this is called after the 'ring' frame (the delta between
 * the last frame and first frame) when one is playing a fli many times.
 *
 * Parameters:
 *		Flifile	*flif;		Open Flifile initialized by pj_fli_open.
 * Returns:
 *		Success (0) if all goes well, a negative error code if not.
 *		(see errcodes.h)
 *************************************************************************/
{
LONG oset;

	if((oset = pj_seek(flif->fd,flif->hdr.frame2_oset,JSEEK_START)) < 0)
		return((Errcode)oset);
}
#endif /* SLUFFED */
