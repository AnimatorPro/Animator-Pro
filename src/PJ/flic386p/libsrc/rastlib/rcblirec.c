#include "rastcall.ih"

void pj_blitrect(Raster *source,			 /* source raster */
			 Coor src_x, Coor src_y,  /* source Minx and Miny */
			 Raster *dest,   		    /* destination raster */
			 Coor dest_x, Coor dest_y, /* destination minx and miny */
			 Coor width, Coor height)  /* blit size */  


/* (should) copys rectangle from source to destination these should handle
 * overlapping by comparing source and dest and branching to routine
 * depending on whether they are the same or not */
{
	if((width = pj_lclip2rects(&src_x,&dest_x,width,
							source->width,dest->width)) <= 0)
		return;
	if((height = pj_lclip2rects(&src_y,&dest_y,height,
							 source->height,dest->height)) <= 0)
		return;

	if(source->type == dest->type)
	{
		(source->lib->blitrect[RL_TO_SAME])(
				 source, src_x, src_y, dest, dest_x, dest_y, width, height);
		return;
	}
	if(dest->type == RT_BYTEMAP)
	{
		(source->lib->blitrect[RL_TO_BYTEMAP])(
				 source, src_x, src_y, dest, dest_x, dest_y, width, height);
		return;
	}
	if(source->type == RT_BYTEMAP)
	{
		(dest->lib->blitrect[RL_FROM_BYTEMAP])(
				 source, src_x, src_y, dest, dest_x, dest_y, width, height);
		return;
	}
	(source->lib->blitrect[RL_TO_OTHER])(
				 source, src_x, src_y, dest, dest_x, dest_y, width, height);
	return;
}
