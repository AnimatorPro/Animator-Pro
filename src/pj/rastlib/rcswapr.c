#include "rastcall.ih"

void pj_swaprect(Raster *rasta,			 /* source raster */
			  Coor rasta_x, Coor rasta_y,    /* source Minx and Miny */
			  Raster *rastb,   		    /* destination raster */
			  Coor rastb_x, Coor rastb_y, /* destination minx and miny */
			  Coor width, Coor height)  /* blit size */  

/* same as above but clipped */
{
	if((width = pj_lclip2rects(&rasta_x,&rastb_x,width,
							rasta->width,rastb->width)) <= 0)
		return;
	if((height = pj_lclip2rects(&rasta_y,&rastb_y,height,
							 rasta->height,rastb->height)) <= 0)
		return;

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
