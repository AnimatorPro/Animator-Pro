#include "rastgfx.ih"

void ublitrect(const Raster *src,	/* source raster */
			  Coor src_x, Coor src_y,	/* source Minx and Miny */
			  const Raster *dest,		/* destination raster */
			  Coor dest_x, Coor dest_y, /* destination minx and miny */
			  Coor width, Coor height,  /* blit size */  
			  const Pixel tcolor)		/* transparent color */

/* Copys rectangle from src to dest where dest is transparent color */
{
Pixel *source_buf, *dest_buf;
Tcolxldat xd;
Pixel sbuf[SBUF_SIZE/sizeof(Pixel)];

	xd.tcolor = tcolor;

	if((width = pj_lclip2rects(&src_x,&dest_x,width,
							src->width,dest->width)) <= 0)
		return;
	if((height = pj_lclip2rects(&src_y,&dest_y,height,
							 src->height,dest->height)) <= 0)
		return;

	source_buf = sbuf;
	if(width > Array_els(sbuf)/2)
	{
		if ((source_buf = pj_malloc(width*(sizeof(Pixel)*2))) == NULL)
			return;
	}

	dest_buf = source_buf + width;
	while(height--)
	{
		GET_HSEG(src,source_buf,src_x,src_y++,width);
		GET_HSEG(dest,dest_buf,dest_x,dest_y,width);
		ubli_line(source_buf, dest_buf, width, &xd);
		PUT_HSEG(dest,dest_buf,dest_x,dest_y++,width);
	}
	if(source_buf != sbuf)
		pj_free(source_buf);
	return;
}
