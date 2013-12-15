#include "rectang.h"

void rect_tofrect(Rectangle *r, Fullrect *fr)
/* copys and/or converts a rectangle to a fullrect source may be destination */
{
	if((Fullrect *)r != fr)
		copy_rectfields(r,fr);

	fr->MaxX = fr->x + fr->width;
	fr->MaxY = fr->y + fr->height;
}
