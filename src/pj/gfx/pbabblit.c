#define PROCBLIT_INTERNALS
#include "rastlib.h"
#include "gfx.h"
#include "memory.h"
#include "errcodes.h"
#include "ptrmacro.h"

Errcode abprocblit(Raster *src,  /* doesn't really need to be a ramrast. */
			Coor src_x, Coor src_y,
			Raster *dest, Coor dest_x, Coor dest_y, Ucoor width, Ucoor height,
		 	Raster *alt, Coor alt_x, Coor alt_y, 
			Procline pline, void *data )
{
Pixel *src_buf, *alt_buf;
Pixel stack_buf[SBUF_SIZE];
Coor diff;

	diff = src_x;
	if((width = pj_lclip2rects(&src_x,&dest_x,width,
											src->width,dest->width)) <= 0)
		return(Err_clipped);
	alt_x += src_x - diff;
	diff  = src_y;
	if((height = pj_lclip2rects(&src_y,&dest_y,height,
											src->height,dest->height)) <= 0)
		return(Err_clipped);
	alt_y += src_y - diff;

	if (src->type == RT_BYTEMAP)
	{
	#define Src ((Bytemap *)src)

		if(width <= Array_els(stack_buf))
			alt_buf = stack_buf;
		else if ((alt_buf = pj_malloc(sizeof(Pixel)*width)) == NULL)
			return(Err_no_memory);

		src_buf = Src->bm.bp[0] + src_y*Src->bm.bpr + src_x;
		while (height--)
		{
			GET_HSEG(alt,alt_buf,alt_x,alt_y++,width);
			(*pline)(src_buf, alt_buf, width, data);
			src_buf += Src->bm.bpr;
			PUT_HSEG(dest,alt_buf,dest_x,dest_y++,width);
		}
	#undef Src
	}
	else
	{
		if(width <= Array_els(stack_buf)/2)
			alt_buf = stack_buf;
		else if ((alt_buf = pj_malloc(2*sizeof(Pixel)*width)) == NULL)
			return(Err_no_memory);
		src_buf = alt_buf+width;

		while(height--)
		{
			GET_HSEG(src,src_buf,src_x,src_y++,width);
			GET_HSEG(alt,alt_buf,alt_x,alt_y++,width);
			(*pline)(src_buf, alt_buf, width, data);
			PUT_HSEG(dest,alt_buf,dest_x,dest_y++,width);
		}
	}
	if(alt_buf != stack_buf)
		pj_free(alt_buf);
	return(Success);
}
