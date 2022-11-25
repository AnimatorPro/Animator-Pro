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

static int
clip1d(int *rwidth, int *rsx, int *rdx, int dwidth)
{
	int dx1 = max(0, *rdx);
	int dx2 = min(dwidth, *rdx + *rwidth);
	int width = dx2 - dx1;

	if (width <= 0)
		return 0;

	*rsx += dx1 - *rdx;
	*rdx = dx1;
	*rwidth = width;
	return 1;
}

int
clipblit_(int *rwidth, int *rheight, int *rsx, int *rsy, int *rdx, int *rdy)
{
	return clip1d(rwidth,  rsx, rdx, WIDTH)
	    && clip1d(rheight, rsy, rdy, HEIGHT);
}

int
clipblit2(int *rwidth, int *rheight,
		int *rsx, int *rsy, int swidth, int sheight,
		int *rdx, int *rdy, int dwidth, int dheight)
{
	return clip1d(rwidth,  rsx, rdx, dwidth)
	    && clip1d(rheight, rsy, rdy, dheight)
	    && clip1d(rwidth,  rdx, rsx, swidth)
	    && clip1d(rheight, rdy, rsy, sheight);
}
