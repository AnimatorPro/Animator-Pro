#include "rastgfx.ih"

Errcode make_cused(Raster *r, UBYTE *c, int max_colors)
/* turns on a byte in a byte array for each pixel value set in a raster */
{
int y,w;
register int count;
register UBYTE *p;
UBYTE *lbuf;

memset(c, FALSE, max_colors);
if ((lbuf = pj_malloc(w = r->width)) == NULL)
	{
	memset(c, TRUE, max_colors);	/* on error set all colors as used */
	return(Err_no_memory);
	}
for(y = 0; y < r->height;++y)
	{
	GET_HSEG(r,lbuf,0,y,w);
	count = w;
	p = lbuf;
	while (--count >= 0)
		c[*p++] = TRUE;
	}
pj_free(lbuf);
return(Success);
}
