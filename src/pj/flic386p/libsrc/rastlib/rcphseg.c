#include "rastcall.ih"

void pj_put_hseg(Raster *v, void *pixbuf, Coor x, Coor y, Ucoor width)
/* Move pixels from memory to a horizontal line of destination raster.
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
	PUT_HSEG(v, pixbuf, x, y, width - x);
	return;
}
