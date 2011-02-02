#include "rectang.h"

void swap_clip(register Cliprect *clip)
/* makes sure x < MaxX and y < MaxY */
{
SHORT swaper;

	if(clip->x > clip->MaxX)
	{
		swaper = clip->x;
		clip->x = clip->MaxX;
		clip->MaxX = swaper;
	}
	if(clip->y > clip->MaxY)
	{
		swaper = clip->y;
		clip->y = clip->MaxY;
		clip->MaxY = swaper;
	}
}
