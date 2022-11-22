#include "rectang.h"

Boolean ptin_rect(register Rectangle *b, SHORT x,SHORT y)
{
	if( ((x -= b->x) < 0)  
		|| (x >= b->width)
		|| ((y -= b->y) < 0)
		|| (y >= b->height))
	{
		return(0);
	}
	return(1);
}
