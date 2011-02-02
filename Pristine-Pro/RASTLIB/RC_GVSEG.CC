#include "rastcall.ih"

void _get_vseg(Raster *r,void *pixbuf,Ucoor x,Ucoor y,Ucoor height)
{
	GET_VSEG(r,pixbuf,x,y,height);
}
