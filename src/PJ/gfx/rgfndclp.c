#include "rastgfx.ih"

Errcode find_clip(void *rast, Rectangle *rect, Pixel tcolor)
/* place minimum boundaries of rast containing all that is not tcolor 
 * into rectangle if nothing is in the rectangle the output width will
 * be zero. */
{
register Raster *r = rast; 
register LONG count;
USHORT bufsize;
UBYTE *pixbuf;
Pixel sbuf[SBUF_SIZE/sizeof(Pixel)];

	rect->width = rect->height = 0;

	/* get one line pixel buffer */

	pixbuf = sbuf;
	bufsize = Max(r->height,r->width);
	if(bufsize > Array_els(sbuf))
	{
		if ((pixbuf = pj_malloc(bufsize*sizeof(Pixel))) == NULL)
			return(Err_no_memory);
	}

	/* figure out the first line in screen with anything in it*/

	count = 0;
	for(;;)
	{
		if(count >= r->height)
			goto done; /* if no pix we went to height all pix the same */ 

		GET_HSEG(r,pixbuf,0,count,r->width); 

		if(pixsame(pixbuf, r->width, tcolor) != r->width)
			break;
		++count;
	}
	rect->y = count;

	/* figure out the last line in screen with anything in it*/

	count = r->height;
	while(--count > rect->y)
	{
		GET_HSEG(r,pixbuf,0,count,r->width);
		if(pixsame(pixbuf, r->width, tcolor) != r->width)
			break;
	}
	rect->height = (count - rect->y) + 1; /* got height */

	/* figure out the first column in screen with anything in it*/
	/* get vertical segment for x (count is x) in y bounds of rect */

	for(count = 0;count < r->width;++count)
	{
		GET_VSEG(r,pixbuf,count,rect->y,rect->height);
		if(pixsame(pixbuf, rect->height, tcolor) != rect->height)
			break;
	}
	rect->x = count;

	/* figure out the last row in screen with anything in it*/

	count = r->width;
	while(--count > rect->x)
	{
	    GET_VSEG(r,pixbuf,count,rect->y,rect->height);
		if(pixsame(pixbuf, rect->height, tcolor) != rect->height)
			break;
	}
	rect->width = (count - rect->x) + 1; /* got width */

done:
	if(pixbuf != sbuf)
		pj_free(pixbuf);
	return(Success);
}
