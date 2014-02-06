#define PROCBLIT_INTERNALS
#include "gfx.h"

void tbli_xlatline(Pixel *source_buf, Pixel *dest_buf, 
		Ucoor width, const Tcolxldat *tcx)
{
Pixel *maxdest;
Pixel tcolor = tcx->tcolor;
Pixel *xlat = tcx->xlat;
Pixel pix;

	maxdest = dest_buf + width;
	while (dest_buf < maxdest)
	{
		if ((pix = *source_buf++) != tcolor)
			*dest_buf++ = xlat[pix];
		else
			++dest_buf;
	}
}
