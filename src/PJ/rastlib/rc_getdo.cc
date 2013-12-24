#include "rastcall.ih"

Pixel _get_dot(Raster *r, Coor x, Coor y)
/* get color of dot */
{
	return(GET_DOT(r,x,y));
}
