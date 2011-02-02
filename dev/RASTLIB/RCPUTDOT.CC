#include "rastcall.ih"

void put_dot(Raster *r, Pixel color, Coor x, Coor y)
/* set dot to color */
{
	CPUT_DOT(r,color,x,y);
}
