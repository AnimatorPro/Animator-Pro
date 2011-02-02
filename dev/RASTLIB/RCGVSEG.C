#include "rastcall.ih"

void pj_get_vseg(Raster *v,void *pixbuf, Coor x,Coor y,Ucoor height)
/* Move pixels from a vertical line of source raster to memory buffer. */
/* (Clipped will only get pixels where buffer overlaps source and 
 * will leave other parts of buffer unaffected) */
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
	GET_VSEG(v, pixbuf, x, y, height - y);
	return;
}
