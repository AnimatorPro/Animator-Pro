#include "rectang.h"

Boolean crects_overlap(register Cliprect *a,register Cliprect *b)
{
	if(    (a->x >= b->MaxX)
		|| (a->y >= b->MaxY)
		|| (a->MaxX <= b->x)
		|| (a->MaxY <= b->y))
	{
		return(0);
	}
	return(1);
}
