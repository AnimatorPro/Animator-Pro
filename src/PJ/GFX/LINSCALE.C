#define GFX_INTERNALS
#include "gfx.h"

void pj_do_linscale(int sx, int sw, int dx, int dw, 
					void (*doinc)(int sx, int dx, void *dat), void *dat )

/* little routine to help with linear scaling functions, calls doinc once for
 * every increment of dx from 0 to dw - 1 with sx ranging from 0 to sw - 1 */
{
int xerr;
int dmax;

	xerr = dw - (sw>>1);
	dmax = dx + dw;

	for(;dx < dmax;++dx)
	{
		doinc(sx,dx,dat);
		if((xerr -= sw) <= 0)
		{
			for(;;)
			{
				++sx;
				if((xerr += dw) > 0)
					break;
			}
		}
	}
}
