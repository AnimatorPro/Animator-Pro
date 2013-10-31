/* cblock_.c */

#include <string.h>
#include "cblock_.h"
#include "clipit_.h"

static int
clipblock(int *rx, int *ry, int *rwidth, int *rheight)
{
	int sx = 0;
	int sy = 0;

	return clipblit_(rwidth, rheight, &sx, &sy, rx, ry);
}

void
xorblock(UBYTE *dst, int x, int y, int width, int height, int col)
{
	if (!clipblock(&x, &y, &width, &height))
		return;

	dst += WIDTH * y + x;

	for (; height > 0; height--) {
		UBYTE *p = dst;
		int i;

		for (i = 0; i < width; i++)
			*p++ ^= col;

		dst += WIDTH;
	}
}

void
cblock(UBYTE *dst, int x, int y, int width, int height, int col)
{
	dst += WIDTH * y + x;

	for (; height > 0; height--) {
		memset(dst, col, width);
		dst += WIDTH;
	}
}

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
