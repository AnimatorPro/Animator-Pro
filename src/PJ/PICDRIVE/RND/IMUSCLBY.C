/***************************************************************
RND file pdr modules:

	Created by Peter Kennard.  Oct 11, 1991
		These modules implement reading scan line and polygon data in
		256 color Autoshade render slide files and drawing the image into
		a screen.
****************************************************************/
#include "stdtypes.h"

int pj_uscale_by(USHORT x, USHORT p, USHORT q)

/* return(x * p/q) done to avoid rounding error */
{
LONG l;

	l = x;
	l *= p;
	l /= q;
	return((int)l);
}
