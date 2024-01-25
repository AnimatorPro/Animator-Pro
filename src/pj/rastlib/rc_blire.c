#include "rastcall.ih"

Errcode
pj__blitrect(Raster *source,			/* source raster */
			 Coor src_x, Coor src_y,  /* source Minx and Miny */
			 Raster *dest,   		    /* destination raster */
			 Coor dest_x, Coor dest_y, /* destination minx and miny */
			 Ucoor width, Ucoor height) /* blit size */

/* (should) copys rectangle from source to destination these should handle
 * overlapping by comparing source and dest and branching to routine
 * depending on whether they are the same or not */
{
	if(source->type == dest->type)
	{
		return (source->lib->blitrect[RL_TO_SAME])(
				 source, src_x, src_y, dest, dest_x, dest_y, width, height);
	}
	if(dest->type == RT_BYTEMAP)
	{
		return (source->lib->blitrect[RL_TO_BYTEMAP])(
				 source, src_x, src_y, dest, dest_x, dest_y, width, height);
	}
	if(source->type == RT_BYTEMAP)
	{
		return (dest->lib->blitrect[RL_FROM_BYTEMAP])(
				 source, src_x, src_y, dest, dest_x, dest_y, width, height);
	}
	return (source->lib->blitrect[RL_TO_OTHER])(
				 source, src_x, src_y, dest, dest_x, dest_y, width, height);
}
