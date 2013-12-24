#include "rastcall.ih"

void _put_hseg(Raster *r,void *pixbuf,Ucoor x,Ucoor y,Ucoor width)
{
	PUT_HSEG(r,pixbuf,x,y,width);
}
