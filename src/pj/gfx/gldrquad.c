#include "gfx.h"

void draw_quad(Raster *r,Pixel color, SHORT minx, SHORT miny, 
					     USHORT width, USHORT height)

/* function for drawing line quadrilatteral */
{
	pj_set_hline(r, color, minx, miny, width);
	pj_set_hline(r, color, minx, miny + height - 1, width);

	height -= 2; /* top and bottom line use up two pixels */
	++miny; /* start just under top line */

	pj_set_vline(r, color, minx, miny, height);
	pj_set_vline(r, color, minx + width - 1, miny, height);
}
