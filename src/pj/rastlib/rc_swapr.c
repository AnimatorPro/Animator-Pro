#include "rastcall.ih"

void pj__swaprect(Raster *rasta,			 /* source raster */
			  Coor rasta_x, Coor rasta_y,    /* source Minx and Miny */
			  Raster *rastb,   		    /* destination raster */
			  Coor rastb_x, Coor rastb_y, /* destination minx and miny */
			  Ucoor width, Ucoor height) /* blit size */

/* unclipped swaps rectangles between raster a and raster b */ 
{
	if(rasta->type == rastb->type)
	{
		(*((rasta)->lib->swaprect[RL_TO_SAME]))(
			 rasta, rasta_x, rasta_y, rastb, rastb_x, rastb_y, width, height);
		return;
	}
	if(rastb->type == RT_BYTEMAP)
	{
		(*((rasta)->lib->swaprect[RL_TO_BYTEMAP]))(
			 rasta, rasta_x, rasta_y, rastb, rastb_x, rastb_y, width, height);
		return;
	}
	if(rasta->type == RT_BYTEMAP)
	{
		(*((rastb)->lib->swaprect[RL_FROM_BYTEMAP]))(
			 rasta, rasta_x, rasta_y, rastb, rastb_x, rastb_y, width, height);
		return;
	}
	(*((rasta)->lib->swaprect[RL_TO_OTHER]))(
		 rasta, rasta_x, rasta_y, rastb, rastb_x, rastb_y, width, height);
	return;
}
