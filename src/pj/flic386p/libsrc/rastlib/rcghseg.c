/*
 * 01-09-91 (Ian) changed parm 'y' to type Coor (was Ucoor).
 */

#include "rastcall.ih"

void pj_get_hseg(Raster *v, void *pixbuf, Coor x, Coor y, Ucoor width)
/* Move pixels from a horizontal line of source raster to memory buffer.
 * this will clip and put pixels in from parts of buffer that overlap
 * destination buffer */
{
	if(((Ucoor)y) >= v->height)
		return;
	if(x >= (Coor)(v->width))
		return;
	if(x < 0)
	{
		width += x;
		pixbuf = OPTR(pixbuf,-x);
		x = 0;
	}
	if(((Coor)width) <= 0)
		return;

	width += x;
	if (width > v->width)
		width = v->width;
	GET_HSEG(v, pixbuf, x, y, width - x);
	return;
}
