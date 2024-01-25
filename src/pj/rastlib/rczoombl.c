
#include "rastcall.ih"
Errcode pj_zoomblit(Raster *source,			/* source raster */
	           Coor src_x, Coor src_y,     /* source Minx and Miny */
	           Raster *dest,   			    /* destination raster */
	           Coor dest_x, Coor dest_y,   /* destination minx and miny */
	           Ucoor width, Ucoor height,  /* destination blit size */  
	           LONG zoom_x, LONG zoom_y )  /* zoom scalers */

/* copy source to destination expanding horizontally by zoom_x 
 * and vertically by zoom_y */
{
	if(source->type == dest->type)
	{
		return (*((source)->lib->zoomblit[RL_TO_SAME]))(
				 source, src_x, src_y, dest, dest_x, dest_y,
		         width, height, zoom_x, zoom_y );
	}
	if(dest->type == RT_BYTEMAP)
	{
		return (*((source)->lib->zoomblit[RL_TO_BYTEMAP]))(
				 source, src_x, src_y, dest, dest_x, dest_y,
		         width, height, zoom_x, zoom_y );
	}
	if(source->type == RT_BYTEMAP)
	{
		return (*((dest)->lib->zoomblit[RL_FROM_BYTEMAP]))(
				 source, src_x, src_y, dest, dest_x, dest_y,
		         width, height, zoom_x, zoom_y );
	}
	return (*((source)->lib->zoomblit[RL_TO_OTHER]))(
		 source, src_x, src_y, dest, dest_x, dest_y,
         width, height, zoom_x, zoom_y );
}
