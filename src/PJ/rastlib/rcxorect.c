#include "rastcall.ih"

void pj_xor_rect( Raster *r, Pixel color,
			   Coor x, Coor y, Ucoor width, Ucoor height )
{
	cliprect(r,x,y,width,height);
	XOR_RECT(r,color,x,y,width,height);
}
