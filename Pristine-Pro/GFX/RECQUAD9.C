#include "rectang.h"

int quad9(SHORT px, SHORT py, SHORT x,SHORT y,SHORT w,SHORT h)
/* figure out which quadrant of rectangle the point is in.  Returns 0-8 */
{
int quad;

	if (w < 0)
	{
		x += w;
		w = -w;
	}
	if (h < 0)
	{
		y += h;
		h = -h;
	}
	if (py < y)
		quad = 0;
	else if (py <= y + h)
		quad = 3;
	else
		quad = 6;

	if (px < x)
		;
	else if (px <= x + w)
		quad += 1;
	else
		quad += 2;

	return(quad);
}
