#include "gfx.h"

void rect_to_xyz(Rectangle *r, Short_xyz *dest)
/* build a rectangular vector polygon from a rectangle */
{
register SHORT *dst;
SHORT x,y,w,h;

	dst = &(dest->x);

	x = r->x;
	y = r->y;
	w = r->width;
	h = r->height;

	w += x-1;
	h += y-1;

	*dst++ = x;
	*dst++ = y;
	*dst++ = 0;

	*dst++ = x;
	*dst++ = h;
	*dst++ = 0;

	*dst++ = w;
	*dst++ = h;
	*dst++ = 0;

	*dst++ = w;
	*dst++ = y;
	*dst++ = 0;
}
