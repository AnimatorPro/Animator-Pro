#include "rastgfx.ih"

int lines_in_blit(Raster *src, Coor src_x, Coor src_y,
				  Raster *dest, Coor dest_x, Coor dest_y, 
				  Ucoor width, Ucoor height)

/* clips and returns count of lines that will be processed by a blit */
{
	if((Coor)(pj_lclip2rects(&src_x,&dest_x,width,src->width,dest->width)) <= 0)
		return(0);
	if((Coor)(height = pj_lclip2rects(&src_y,&dest_y,height,
											src->height,dest->height)) <= 0)
		return(0);
	return(height);
}
