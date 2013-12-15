#include "rectang.h"

Boolean and_cliprects(register Cliprect *a,register Cliprect *b,Cliprect *out)

/* returns 1 if rectangles intersect and out is loaded with union.
 * If no intersection occurrs returns 0. */
{
	if(    (a->x >= b->MaxX)
		|| (a->y >= b->MaxY)
		|| (a->MaxX <= b->x)
		|| (a->MaxY <= b->y))
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

	if(a->MaxX < b->MaxX)
		out->MaxX = a->MaxX;
	else
		out->MaxX = b->MaxX;

	if(a->MaxY < b->MaxY)
		out->MaxY = a->MaxY;
	else
		out->MaxY = b->MaxY;

	return(1);
}
