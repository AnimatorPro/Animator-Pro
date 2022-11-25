#define PROCBLIT_INTERNALS
#include "gfx.h"


void ubli_xlatline(Pixel *source_buf, Pixel *dest_buf, 
		Ucoor width, const Tcolxldat *tcx)
{
Pixel *maxdest;
Pixel tcolor = tcx->tcolor;
Pixel *xlat = tcx->xlat;

	maxdest = dest_buf + width;
	while (dest_buf < maxdest)
	{
		if (*dest_buf == tcolor)
			*dest_buf++ = xlat[*source_buf++];
		else
		{
			++dest_buf;
			++source_buf;
		}
	}
}
