#include "rastcall.h"

void _put_vseg(Raster *r,void *pixbuf, Ucoor x,Ucoor y,Ucoor height)
{
	PUT_VSEG(r,pixbuf,x,y,height);
}
