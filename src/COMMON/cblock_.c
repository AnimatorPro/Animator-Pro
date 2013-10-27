/* cblock_.c */

#include "cblock_.h"

void
cdot(UBYTE *dst, int x, int y, int col)
{
	if ((0 < x && x < WIDTH) && (0 < y && y < HEIGHT))
		dst[WIDTH * y + x] = col;
}
