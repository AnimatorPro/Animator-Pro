#define PROCBLIT_INTERNALS
#include "gfx.h"

void ubli_line(Pixel *source_buf, Pixel *dest_buf, 
			   Coor width, const Tcolxldat *tcx)
/* (Private to grc_driver.) */
{
Pixel *maxdest;
Pixel tcolor = tcx->tcolor;

	maxdest = dest_buf + width;
	while (dest_buf < maxdest)
	{
		if (*dest_buf == tcolor)
			*dest_buf++ = *source_buf++;
		else
		{
			++dest_buf;
			++source_buf;
		}
	}
}
