/* compbrun.c */

#define RASTCOMP_INTERNALS
#include "ptrmacro.h"
#include "rastcomp.h"

void *pj_brun_rect(Raster *r,void *cbuf,
				SHORT x,SHORT y,USHORT width,USHORT height)

/* brun compresses all pixels in a raster rectangle and puts them in cbuf
 * it returns length of buffer used in cbuf 0 if overflow */
{
register char *c;
register int bpr;
char *cmax;
UBYTE *lbuf;

	c = cbuf;
	cmax = c + (width * height);

	if(r->type == RT_BYTEMAP) /* if we can get'um directly do it fast way */
	{
		bpr = ((Bytemap *)r)->bm.bpr;
		/* note (y - 1): pre-increment below */
		lbuf = ((Bytemap *)r)->bm.bp[0] + (bpr * (y - 1)); 
	}
	else
	{
		bpr = 0;
		lbuf = (UBYTE *)cmax;
	}

	while(height--)
	{
		if(bpr)
			lbuf += bpr;
		else
			pj_get_hseg(r,lbuf,x,y++,width);

		c = pj_brun_comp_line(lbuf,c,width);
		if(c >= cmax)
			return(NULL);
	}
	return(c);
}

