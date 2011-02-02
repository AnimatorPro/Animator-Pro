#include "rastcall.ih"

/* Since this isn't clipped should it have a return value??? */
void _get_hseg(Raster *r,void *pixbuf,Ucoor x,Ucoor y,Ucoor width)
{
	GET_HSEG(r,pixbuf,x,y,width);
}
