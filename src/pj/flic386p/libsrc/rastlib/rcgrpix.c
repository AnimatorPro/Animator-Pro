#include "rastcall.ih"

void pj_get_rectpix(Raster *r,void *pixbuf,
 		   		 Coor x,Coor y,Ucoor width,Ucoor height)

/* loads all pixels from a screen rectangle into a linear buffer
 * on a line by line basis.
 * If the rectangle is clipped garbage will
 * be in the clipped off section of the buffer */
{
	(r->lib->get_rectpix)((Raster *)r,
					(void *)pixbuf,(Coor)x,(Coor)y,
					(Ucoor)width,(Ucoor)height);
}
