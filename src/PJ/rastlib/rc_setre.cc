#include "rastcall.ih"

void _set_rect(Raster *p, Pixel color,
			  Coor x, Coor y, Ucoor width, Ucoor height )
{
	SET_RECT(p,color,x,y,width,height);
}
