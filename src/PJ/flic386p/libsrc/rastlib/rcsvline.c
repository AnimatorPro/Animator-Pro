#include "rastcall.ih"

void pj_set_vline(Raster *v, Pixel color, Coor x, Coor y, Ucoor height)
/* Draw a solid vertical line. */
/* (Clipped) */
{
	if(((Ucoor)x) >= v->width)
		return;
	if(y >= (Coor)(v->height))
		return;
	if(y < 0)
	{
		height += y;
		y = 0;
	}
	if(((Coor)height) <= 0)
		return;
	height += y;
	if (height > v->height)
		height = v->height;
	SET_VLINE(v, color, x, y, height - y);
}
