#include "rastcall.ih"

void _mask2blit(UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
			   Raster *r, Coor rx, Coor ry, Ucoor width, Ucoor height,
			   Pixel oncolor, Pixel offcolor )

/* sets rectangle of raster rectangle of mask 
	(mask on = oncolor, off = noaction ) */
{
	MASK2BLIT(mbytes,mbpr,mx,my,r,rx,ry,width,height,oncolor,offcolor);
}
