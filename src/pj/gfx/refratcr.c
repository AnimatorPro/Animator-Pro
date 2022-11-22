#include "rectang.h"

void frame_tocrect(SHORT x0, SHORT y0, SHORT x1, SHORT y1, Cliprect *cr)

/* takes two x,y pairs forming the corners of a box and puts them in a
 * cliprect using its conventions */
{
	cr->x = x0;
	cr->y = y0;
	cr->MaxX = x1;
	cr->MaxY = y1;
	swap_clip(cr);
	++cr->MaxX;
	++cr->MaxY;
}
