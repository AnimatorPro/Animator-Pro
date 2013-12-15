#include "rastcall.ih"

void unbrun_rect(Raster *r,void *ucbuf, LONG pixsize,
				 Coor x,Coor y,Ucoor width,Ucoor height)
{
	(r->lib->unbrun_rect)(r,ucbuf,pixsize,x,y,width,height);
}
