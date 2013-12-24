#include "rastcall.ih"

void pj_tblitrect(Raster *source,			 /* source raster */
			   Coor src_x, Coor src_y,  /* source Minx and Miny */
			   Raster *dest,   		    /* destination raster */
			   Coor dest_x, Coor dest_y, /* destination minx and miny */
			   Coor width, Coor height,  /* blit size */  
		  	   Pixel tcolor )             /* color to ignore in source */

/* copy all of source that is not tcolor to destination */
{
	if((width = pj_lclip2rects(&src_x,&dest_x,width,
							source->width,dest->width)) <= 0)
		return;
	if((height = pj_lclip2rects(&src_y,&dest_y,height,
							 source->height,dest->height)) <= 0)
		return;

	if(source->type == dest->type)
	{
		(source->lib->tblitrect[RL_TO_SAME])(
		 source, src_x, src_y, dest, dest_x, dest_y, width, height, tcolor);
		return;
	}
	if(dest->type == RT_BYTEMAP)
	{
		(source->lib->tblitrect[RL_TO_BYTEMAP])(
		 source, src_x, src_y, dest, dest_x, dest_y, width, height, tcolor);
		return;
	}
	if(source->type == RT_BYTEMAP)
	{
		(dest->lib->tblitrect[RL_FROM_BYTEMAP])(
		 source, src_x, src_y, dest, dest_x, dest_y, width, height, tcolor);
		return;
	}
	(source->lib->tblitrect[RL_TO_OTHER])(
		 source, src_x, src_y, dest, dest_x, dest_y, width, height, tcolor);
	return;
}
