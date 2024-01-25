#include "rectang.h"

void crect_torect(register Cliprect *cr, register Rectangle *r)

/* copys and or converts a cliprect to a rectangle */
{
	r->x = cr->x;
	r->width = cr->MaxX - cr->x;
	r->y = cr->y;
	r->height = cr->MaxY - cr->y;
}
