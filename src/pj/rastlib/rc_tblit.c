#include "rastcall.ih"

Errcode pj__tblitrect(Raster *source,			 /* source raster */
			   Coor src_x, Coor src_y,  /* source Minx and Miny */
			   Raster *dest,   		    /* destination raster */
			   Coor dest_x, Coor dest_y, /* destination minx and miny */
			   Ucoor width, Ucoor height, /* blit size */
		  	   Pixel tcolor )             /* color to ignore in source */

/* copy all of source that is not tcolor to destination */
{
	if(source->type == dest->type)
	{
		return (source->lib->tblitrect[RL_TO_SAME])(
		 source, src_x, src_y, dest, dest_x, dest_y, width, height, tcolor);
	}
	if(dest->type == RT_BYTEMAP)
	{
		return (source->lib->tblitrect[RL_TO_BYTEMAP])(
		 source, src_x, src_y, dest, dest_x, dest_y, width, height, tcolor);
	}
	if(source->type == RT_BYTEMAP)
	{
		return (dest->lib->tblitrect[RL_FROM_BYTEMAP])(
		 source, src_x, src_y, dest, dest_x, dest_y, width, height, tcolor);
	}
	return (source->lib->tblitrect[RL_TO_OTHER])(
		 source, src_x, src_y, dest, dest_x, dest_y, width, height, tcolor);
}
