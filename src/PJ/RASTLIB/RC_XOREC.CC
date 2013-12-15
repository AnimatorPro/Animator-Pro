#include "rastcall.ih"

void _xor_rect( Raster *r, Pixel color,
			   Coor x, Coor y, Ucoor width, Ucoor height )
{
	XOR_RECT(r,color,x,y,width,height);
}
