#include "rastcall.ih"

Pixel get_dot(Raster *r, Coor x, Coor y)
/* get color of dot */
{
	return(CGET_DOT(r,x,y));
}
