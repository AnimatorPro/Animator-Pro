/* clipit_.c */

#include "jimk.h"
#include "clipit_.h"

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

int
clipblit_(int *rwidth, int *rheight, int *rsx, int *rsy, int *rdx, int *rdy)
{
	int dx1 = max(0, *rdx);
	int dx2 = min(WIDTH, *rdx + *rwidth);
	int dy1 = max(0, *rdy);
	int dy2 = min(HEIGHT, *rdy + *rheight);
	int width = dx2 - dx1;
	int height = dy2 - dy1;

	if (width <= 0 || height <= 0)
		return 0;

	*rsx += dx1 - *rdx;
	*rsy += dy1 - *rdy;
	*rdx = dx1;
	*rdy = dy1;
	*rwidth = width;
	*rheight = height;
	return 1;
}
