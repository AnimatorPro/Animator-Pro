#include "rastcall.ih"

void unss2_rect(Raster *r,void *ucbuf, LONG pixsize,
				   Coor x,Coor y,Ucoor width,Ucoor height)
{
	(r->lib->unss2_rect)(r,ucbuf,pixsize,x,y,width,height);
}
