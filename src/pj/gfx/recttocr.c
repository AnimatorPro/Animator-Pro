#include "rectang.h"

void rect_tocrect(register Rectangle *r,register Cliprect *cr)

/* copys and converts a cliprect to a rectangle */
{
	cr->x = r->x;
	cr->MaxX = cr->x + r->width;
	cr->y = r->y;
	cr->MaxY = cr->y + r->height;
}
