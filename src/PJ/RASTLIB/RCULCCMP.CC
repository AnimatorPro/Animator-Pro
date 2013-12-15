#include "rastcall.ih"

void unlccomp_rect(Raster *r,void *ucbuf, LONG pixsize,
				   Coor x,Coor y,Ucoor width,Ucoor height)
{
	(r->lib->unlccomp_rect)(r,ucbuf,pixsize,x,y,width,height);
}
