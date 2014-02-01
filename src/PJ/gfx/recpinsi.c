#include "rectang.h"

Boolean ptinside_rect(Rectangle *b, SHORT x, SHORT y, SHORT inside)
{
	if( ((x -= b->x) < inside)  
		|| (x >= b->width-inside)
		|| ((y -= b->y) < inside)
		|| (y >= b->height-inside))
	{
		return(0);
	}
	return(1);
}
