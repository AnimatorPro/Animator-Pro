#define PROCBLIT_INTERNALS
#include "rastlib.h"
#include "gfx.h"
#include "memory.h"
#include "errcodes.h"
#include "ptrmacro.h"

Errcode xlatblit(Raster *src,			 /* source raster */
		        Coor src_x, Coor src_y,  /* source Minx and Miny */
		        Raster *dest,   		 /* destination raster */
		  		Coor dest_x, Coor dest_y, /* destination minx and miny */
			    Ucoor width, Ucoor height, /* blit size */  
				Pixel *ttable )

/* translates a rectangular area from src raster to dest raster where src
    and dest may be any type of raster. The process function only has 
	access to the source buffer it may be used to translate a rectangle in 
	a raster by setting source and dest the same */
{
Pixel *lbuf;
Pixel stack_buf[SBUF_SIZE];

	if((width = pj_lclip2rects(&src_x,&dest_x,width,
								    src->width,dest->width)) <= 0)
		return(Err_clipped);
	if((height = pj_lclip2rects(&src_y,&dest_y,height,
									src->height,dest->height)) <= 0)
		return(Err_clipped);

	if(width <= Array_els(stack_buf))
		lbuf = stack_buf;
	else if ((lbuf = pj_malloc(width*sizeof(Pixel))) == NULL)
		return(Err_no_memory);

	while(height--)
	{
		GET_HSEG(src,lbuf,src_x,src_y++,width);
		pj_xlate(ttable,lbuf,width);
		PUT_HSEG(dest,lbuf,dest_x,dest_y++,width);
	}
	if(lbuf != stack_buf)
		pj_free(lbuf);
	return(Success);
}
