/* cblock_.c */

#include <string.h>
#include "cblock_.h"

void
chli(UBYTE *dst, int x, int y, int width, int col)
{
	dst += WIDTH * y + x;
	memset(dst, col, width);
}

void
cvli(UBYTE *dst, int x, int y, int height, int col)
{
	dst += WIDTH * y + x;

	for (; height > 0; height--) {
		*dst = col;
		dst += WIDTH;
	}
}

void
cdot(UBYTE *dst, int x, int y, int col)
{
	if ((0 < x && x < WIDTH) && (0 < y && y < HEIGHT))
		dst[WIDTH * y + x] = col;
}
