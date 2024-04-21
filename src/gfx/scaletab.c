#include "gfx.h"

void pj_make_scale_table(int sw, int dw, SHORT *stable)

/* Builds a table for scaling a line of items sw wide to a line of items dw
 * wide.  Each entry in stable will have the index of the source element to put
 * into the dest element coresponding with the stable entry. The stable must
 * have at least dsize elements. If dw < 0 it will build the table backwards.
 * sw must always be positive or this may hang.
 *
 * To use:  for(x = 0 to last)
 *				dst[x] = src[stable[x]];
 */
{
int xerr;
int sx, dx;
int sinc;

	if(dw < 0)
	{
		sinc = -1;
		dw = -dw;
		sx = sw - 1;
	}
	else
	{
		sx = 0;
		sinc = 1;
	}

	xerr = dw - (sw>>1);
	for(dx = 0;dx < dw;++dx)
	{
		*stable++ = sx;
		if((xerr -= sw) <= 0)
		{
			for(;;)
			{
				sx += sinc;
				if((xerr += dw) > 0)
					break;
			}
		}
	}
}
