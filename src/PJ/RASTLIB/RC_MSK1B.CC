#include "rastcall.ih"

void _mask1blit(UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
			   Raster *r, Coor rx, Coor ry, Ucoor width, Ucoor height,
			   Pixel oncolor )
{
	MASK1BLIT(mbytes,mbpr,mx,my,r,rx,ry,width,height,oncolor);
}
