#include "rectang.h"

Boolean and_rects(register Rectangle *a,register Rectangle *b,Rectangle *out)

/* returns 1 if rectangles intersect and out is loaded with union.
 * If no intersection occurrs returns 0. out may be a or b */
{
SHORT amaxx, bmaxx;
SHORT amaxy, bmaxy;

	if(    (a->x >= (bmaxx = b->x + b->width))
		|| (a->y >= (bmaxy = b->y + b->height))
		|| ((amaxx = a->x + a->width) <= b->x)
		|| ((amaxy = a->y + a->height) <= b->y))
	{
		return(0);
	}

	if(a->x > b->x)
		out->x = a->x;
	else
		out->x = b->x;

	if(a->y > b->y)
		out->y = a->y;
	else
		out->y = b->y;

	if(amaxx < bmaxx)
		out->width = amaxx;
	else
		out->width = bmaxx;

	if(amaxy < bmaxy)
		out->height = amaxy;
	else
		out->height = bmaxy;

	out->width -= out->x;
	out->height -= out->y;

	return(1);
}
