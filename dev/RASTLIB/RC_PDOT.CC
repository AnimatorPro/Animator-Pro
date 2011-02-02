#include "rastcall.ih"

void _put_dot(Raster *r, Pixel color, Coor x, Coor y)
/* set dot to color */
{
	PUT_DOT(r,color,x,y);
}
