#include "gfx.ih"

void line(void *r, Pixel color, Coor x1, Coor y1, Coor x2, Coor y2)

/* for simple solid colored lines */
{
Sdat sd;

	sd.rast = r;
	sd.color = color;

	pj_cline(x1, y1, x2, y2, sdot, &sd);
}
