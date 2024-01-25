/* grcfulib.c - Contains all remaining calls not contained in the
   comp calls dcomp calls or common calls and a function to load
   all of them into a fully loaded library
   
   	   Generic display driver for hi-res animator.
   Useful to fill in parts of a driver that are not implemented.
   Requires you to fill in _get_dot and _put_dot.  The rest of
   the system will funnel through these.  Over-ride other
   functions for increased performance.  Does reasonably well 
   speedwise if _get_hseg(), _put_hseg(), and pj__set_hline() are 
   implemented as higher level functions such as the blits go through
   these. */

#define GRCLIB_C
#include "errcodes.h"
#include "ptrmacro.h"
#include "memory.h"
#include "rastcall.h"
#include "rastlib.h"


static void _grc_cput_dot(Raster *r, Pixel color, Coor x, Coor y)
/* Set a single pixel. */
/* (Clipped) */
{
	if (((Ucoor)x) >= r->width || ((Ucoor)y) >= r->height)
		return;
	PUT_DOT(r,color,x,y);
}

static Pixel _grc_cget_dot(Raster *v, Coor x, Coor y)
/* Get the value of a single pixel. */
/* (Clipped) */
{
if ((Ucoor)x >= v->width || (Ucoor)y >= v->height)
	return(0);
return(GET_DOT(v,x,y));
}

static Errcode grc_blitrect(Raster *src,			 /* source raster */
				  Coor src_x, Coor src_y,  /* source Minx and Miny */
				  Raster *dest,   		     /* destination raster */
				  Coor dest_x, Coor dest_y, /* destination minx and miny */
				  Ucoor width, Ucoor height) /* blit size */
/* Copy rectangular area from src raster to dest raster where src
   and dest may be any type of raster. */
/* (Private to grc_driver (though exported).) */
{
Pixel *lbuf;
Pixel sbuf[SBUF_SIZE/sizeof(Pixel)];


	lbuf = sbuf;
	if(width > Array_els(sbuf))
	{
		if ((lbuf = pj_malloc(width*sizeof(Pixel))) == NULL)
			return(Err_no_memory);
	}

	while(height--)
	{
		GET_HSEG(src,lbuf,src_x,src_y++,width);
		PUT_HSEG(dest,lbuf,dest_x,dest_y++,width);
	}
	if(lbuf != sbuf)
		pj_free(lbuf);
	return(Success);
}

static Errcode grc_f_blitrect(Raster *source, /* source raster */
				  Coor src_x, Coor src_y,  /* source Minx and Miny */
				  Raster *dest,   		     /* destination raster */
				  Coor dest_x, Coor dest_y, /* destination minx and miny */
				  Ucoor width, Ucoor height) /* blit size */
/* Copies from bytemap source to generic destination */
{
Bytemap *src = (Bytemap *)source;
Pixel *lbuf;

	lbuf = src->bm.bp[0] + (src->bm.bpr*src_y) + src_x;
	while(height--)
	{
		PUT_HSEG(dest,lbuf,dest_x,dest_y++,width);
		lbuf += src->bm.bpr;
	}

	return Success;
}

static Errcode grc_t_blitrect(Raster *src, /* source raster */
				  Coor src_x, Coor src_y,  /* source Minx and Miny */
				  Raster *dst, /* destination raster */
				  Coor dest_x, Coor dest_y, /* destination minx and miny */
				  Ucoor width, Ucoor height) /* blit size */
/* Copies rectangle to bytemap destination from driver-type source. */
/* (Clips to make sure rectangle lies inside destination.) */
{
Bytemap *dest = (Bytemap *)dst;
Pixel *lbuf;

	lbuf = dest->bm.bp[0] + (dest->bm.bpr*dest_y) + dest_x;
	while(height--)
	{
		GET_HSEG(src,lbuf,src_x,src_y++,width);
		lbuf += dest->bm.bpr;
	}

	return Success;
}

void pj_grc_load_commcalls(struct rastlib *lib)
/* Return pointer to generic display function jump-table */
{
	lib->cput_dot = _grc_cput_dot;
	lib->cget_dot = _grc_cget_dot;

	lib->blitrect[RL_TO_SAME] = grc_blitrect;
	lib->blitrect[RL_TO_BYTEMAP] = grc_t_blitrect;
	lib->blitrect[RL_FROM_BYTEMAP] = grc_f_blitrect;
	lib->blitrect[RL_TO_OTHER] = grc_blitrect;
}
