#include "rastgfx.ih"

void xlat_rast(Raster *r, UBYTE *ttable, LONG pixsize)

/* translates pixels by indexing them into pixel translation table ttable 
 * pixsize is the pixel size in the ttable. */
{
#define BM ((Bytemap*)r) 
LONG x,y;
LONG get_width, width_left;
LONG numpix;
UBYTE lbuf[SBUF_SIZE];
(void)pixsize;

	if(r->type == RT_BYTEMAP)
	{
		pj_xlate(ttable,BM->bm.bp[0],BM->bm.psize);
		return;
	}

	get_width = r->width;
	if(get_width > sizeof(lbuf))
		get_width = sizeof(lbuf);

	y = r->height;

	for(y = 0; y < r->height;++y)
	{
		width_left = r->width;
		numpix = get_width;
		x = 0;
		for(;;)
		{
			GET_HSEG(r,lbuf,x,y,numpix);
			pj_xlate(ttable,lbuf,numpix);
			PUT_HSEG(r,lbuf,x,y,numpix);
			if((width_left -= numpix) <= 0)
				break;
			x += numpix;
			if(width_left < numpix)
				numpix = width_left;
		}
	}
#undef BM 
}
