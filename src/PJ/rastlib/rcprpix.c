#include "rastcall.ih"

void
_pj_put_rectpix(Raster *r, Pixel *pixbuf,
		Coor x, Coor y, Ucoor width, Ucoor height)
/* loads all pixels from a linear pixel buffer into a screen rectangle */ 
{
	(r->lib->put_rectpix)((Raster *)r,
					(void *)pixbuf,(Coor)x,(Coor)y,
					(Ucoor)width,(Ucoor)height);
}
