#include "rastgfx.ih"

void blitmove_rect(Raster *s,Coor sx, Coor sy, Raster *d,
				   Coor dx, Coor dy, Coor width, Coor height )

/* like a pj_blitrect but will blit to another position in same raster 
 * without overwriting itself */
{
Coor xdif, ydif;
Coor inc;
Pixel *lbuf;
Pixel sbuf[SBUF_SIZE/sizeof(Pixel)];


	if((width = pj_lclip2rects(&sx,&dx,width,s->width,d->width)) <= 0)
		return;
	if((height = pj_lclip2rects(&sy,&dy,height,s->height,d->height)) <= 0)
		return;

	if(s != d)
		goto blitit;
	xdif = sx - dx;
	ydif = sy - dy;

	if(!xdif && !ydif)
		return;

	if( xdif >= width
	    || -xdif >= width)
	{
		goto blitit;
	}

	if( ydif >= height
	    || -ydif >= height)
	{
		goto blitit;
	}
	
	if(sy >= dy) /* top down */
		inc = 1;
	else  /* bottom up */
	{
		sy += height - 1;
		dy += height - 1;
		inc = -1;
	}

	lbuf = sbuf;
	if(width > Array_els(sbuf))
	{
		if ((lbuf = pj_malloc(width*sizeof(Pixel))) == NULL)
			return;
	}

	while(height--)
	{
		GET_HSEG(s,lbuf,sx,sy,width);
		PUT_HSEG(d,lbuf,dx,dy,width);
		sy += inc;
		dy += inc;
	}

	if(lbuf != sbuf)
		pj_free(lbuf);
	return;
blitit:
	pj__blitrect(s,sx,sy,d,dx,dy,width,height);
	return;
}
