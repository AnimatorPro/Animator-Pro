#include "rastcall.ih"

void pj_put_vseg(Raster *v,void *pixbuf, Coor x,Coor y,Ucoor height)
/* Move pixels from memory to a vertical line of destination raster. */
/* (Clipped will put pixels in destination that overlap) */
{
	if(y >= (Coor)(v->height))
		return;
	if(((Ucoor)x) >= v->width)
		return;
	if(y < 0)
	{
		height += y;
		pixbuf = OPTR(pixbuf,-y);
		y = 0;
	}
	if(((Coor)height) <= 0)
		return;

	height += y;
	if (height > v->height)
		height = v->height;
	PUT_VSEG(v, pixbuf, x, y, height - y);
	return;
}
