/* truecol_.c */

#include "truecol_.h"

static int
max(int a, int b)
{
	return (a >= b) ? a : b;
}

static int
min(int a, int b)
{
	return (a <= b) ? a : b;
}

static int
clamp(int a, int b, int c)
{
	return min(max(a, b), c);
}

unsigned int
closestc(const UBYTE *rgb, const UBYTE *cmap, int count)
{
	const int r = rgb[0];
	const int g = rgb[1];
	const int b = rgb[2];
	unsigned int closest_diff2 = 32000;
	unsigned int closest_index = 0;
	int i = 0;

	for (i = 0; i < count; i++) {
		int dr = (cmap[3 * i + 0] - r);
		int dg = (cmap[3 * i + 1] - g);
		int db = (cmap[3 * i + 2] - b);
		unsigned int diff2 = dr*dr + dg*dg + db*db;

		if (diff2 < closest_diff2) {
			closest_index = i;
			closest_diff2 = diff2;
		}
	}

	return closest_index;
}

void
colorave(int x, int y, UBYTE *rgb, const UBYTE *screen, const UBYTE *cmap)
{
	int i, j, n;
	UBYTE r[9];
	UBYTE g[9];
	UBYTE b[9];

	n = 0;
	for (i = y - 1; i <= y + 1; i++) {
		int cy = clamp(0, i, HEIGHT - 1);

		for (j = x - 1; j <= x + 1; j++) {
			int cx = clamp(0, j, WIDTH - 1);

			r[n] = cmap[3 * screen[WIDTH * cy + cx] + 0];
			g[n] = cmap[3 * screen[WIDTH * cy + cx] + 1];
			b[n] = cmap[3 * screen[WIDTH * cy + cx] + 2];
			n++;
		}
	}

	rgb[0] = ( 1*r[0] + 2*r[1] + 1*r[2]
	         + 2*r[3] + 4*r[4] + 2*r[5]
	         + 1*r[6] + 2*r[7] + 1*r[8]
	         + 8) / 16;
	rgb[1] = ( 1*g[0] + 2*g[1] + 1*g[2]
	         + 2*g[3] + 4*g[4] + 2*g[5]
	         + 1*g[6] + 2*g[7] + 1*g[8]
	         + 8) / 16;
	rgb[2] = ( 1*b[0] + 2*b[1] + 1*b[2]
	         + 2*b[3] + 4*b[4] + 2*b[5]
	         + 1*b[6] + 2*b[7] + 1*b[8]
	         + 8) / 16;
}
