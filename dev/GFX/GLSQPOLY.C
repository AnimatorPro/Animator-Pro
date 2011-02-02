#include "gfx.h"

void sq_poly(SHORT w, SHORT h, SHORT x, SHORT y, Short_xy *dest_points)
/* sq_poly - make up a 4 element polygon from a rectangle */
{
register SHORT *dest;

if(w > 0)
	w += x-1;
else
	w += x+1;

if(h > 0)
	h += y-1;
else
	h += y+1;

dest = &(dest_points->x); /* start with first one */

*dest++ = x;
*dest++ = y;

*dest++ = x;
*dest++ = h;

*dest++ = w;
*dest++ = h;

*dest++ = w;
*dest++ = y;
}
