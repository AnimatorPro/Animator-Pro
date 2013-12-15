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
#include "libdummy.h"


static void _grc_put_vseg(const Raster *v,Pixel *pixbuf,
	Ucoor x,Ucoor y,Ucoor height)
/* Move pixels from memory to a vertical line of destination raster. */
/* (Unclipped) */
{
while (height-- > 0)
	PUT_DOT(v, *pixbuf++, x, y++);
}

static void _grc_get_vseg(const Raster *v,Pixel *pixbuf,
	Ucoor x,Ucoor y,Ucoor height)
/* Move pixels from a vertical line of source raster to memory buffer. */
/* (Unclipped) */
{
while (height-- > 0)
	*pixbuf++ = GET_DOT(v, x, y++);
}

static void _grc_set_vline(const Raster *v, Pixel color, 
	Ucoor x, Ucoor y, Ucoor height)
/* Draw a solid vertical line. */
/* (Unclipped) */
{
while (height-- > 0)
	PUT_DOT(v, color, x, y++);
}

static void _grc_xor_rect(const Raster *v, Pixel color, 
	Ucoor x, Ucoor y, Ucoor width, Ucoor height)
/* Xor a rectangular piece of the raster with color. */
/* (Unclipped) */
{
Ucoor x1, w1;
Pixel sbuf[SBUF_SIZE/sizeof(Pixel)];
Pixel *lbuf;


lbuf = sbuf;
if(width > Array_els(sbuf))
	{
	if ((lbuf = pj_malloc(width*sizeof(Pixel))) == NULL)
		goto SLOW;
	}
while (height-- > 0)
	{
	GET_HSEG(v,lbuf,x,y,width);
	pj_xor_bytes(color,lbuf,width);
	PUT_HSEG(v,lbuf,x,y++,width);
	}
if (lbuf != sbuf)
	pj_free(lbuf);
return;
SLOW:
while (height-- > 0)
	{
	x1 = x;
	w1 = width;
	while (w1-- > 0)
		{
		PUT_DOT(v, color^GET_DOT(v,x1,y), x1, y);
		x1++;
		}
	y++;
	}
}

#ifdef UNTESTED
static void mask1line(UBYTE *mbytes, UBYTE bit1, Pixel *lbuf, Coor width,
					 const Pixel oncolor)

/* Expand an array of bits in memory into pixel buffer.  Where there are
   1's in the source bit-array set to oncolor. */
/* (Private to grc_driver.) */
{
UBYTE byte;

	byte = *mbytes++;
	while(--width >= 0)
	{
		if(bit1 & byte)
			*lbuf++ = oncolor;

		if((bit1>>=1) == 0)
		{
			bit1 = 0x80;
			byte = *mbytes++;
		}
	}
}
static void _grc_mask1blit(UBYTE *mbytes, const unsigned mbpr, 
 	Coor src_x, Coor src_y, 
	const Raster *dest, 
	Coor dest_x, Coor dest_y,
	Coor width, Coor height, 
	const Pixel oncolor)
/* Expand a memory buffer arranged as a bit-plane into a rectangular
   area of dest raster.  Used to implement graphics text and icons.
   1's in bit-plane are set to oncolor in dest.  
   0's in bit-plane leave dest unchanged. */
/* (Unclipped.) */
{
Pixel *lbuf;
Pixel sbuf[SBUF_SIZE/sizeof(Pixel)];
UBYTE bit1;

	lbuf = sbuf;
	if(width > Array_els(sbuf))
	{
		if((lbuf = pj_malloc(width*sizeof(Pixel))) == NULL)
			return;
	}

	bit1 = (0x80>>(src_x&7));
	mbytes += (src_x>>3) + src_y*mbpr;

	while(--height >= 0)
	{
		GET_HSEG(dest,lbuf,dest_x,dest_y,width);
		mask1line(mbytes,bit1,lbuf,width,oncolor);
		PUT_HSEG(dest,lbuf,dest_x,dest_y++,width);
		mbytes += mbpr;
	}
	if(lbuf != sbuf)
		pj_free(lbuf);
}
#endif /* UNTESTED */

static mask1line(UBYTE *mbytes, UBYTE bit1, Coor width,
	const Raster *dest, Coor dest_x, const Coor dest_y, 
	const Pixel oncolor)
/* Expand an array of bits in memory into dest raster.  Where there are
   0's in the source bit-array leave the raster as is.
   1's in the source bit-array set raster to oncolor. */
/* (Private to grc_driver.) */
{
UBYTE byte;

byte = *mbytes++;
while (--width >= 0)
	{
	if (bit1 & byte)
		PUT_DOT(dest, oncolor, dest_x, dest_y);
	dest_x++;
	if ((bit1>>=1) == 0)
		{
		bit1 = 0x80;
		byte = *mbytes++;
		}
	}
}

static void _grc_mask1blit(UBYTE *mbytes, const unsigned mbpr, 
 	Coor src_x, Coor src_y, 
	const Raster *dest, 
	Coor dest_x, Coor dest_y,
	Coor width, Coor height, 
	const Pixel oncolor)
/* Expand a memory buffer arranged as a bit-plane into a rectangular
   area of dest raster.  Used to implement graphics text and icons.
   1's in bit-plane are set to oncolor in dest.  
   0's in bit-plane leave dest unchanged. */
/* (Unclipped.) */
{
UBYTE bit1;

bit1 = (0x80>>(src_x&7));
mbytes += (src_x>>3) + src_y*mbpr;
while (--height >= 0)
	{
	mask1line(mbytes, bit1, width, dest, dest_x, dest_y, oncolor);
	mbytes += mbpr;
	++src_y;
	++dest_y;
	}
}


static void mask2line(UBYTE *mbytes, UBYTE bit1, Pixel *lbuf, Coor width,
					 const Pixel oncolor, const Pixel offcolor)

/* Expand an array of bits in memory into pixel buffer.  Where there are
   0's in the source bit-array set to offcolor.
   1's in the source bit-array set to oncolor. */
/* (Private to grc_driver.) */
{
UBYTE byte;

	byte = *mbytes++;
	while(--width >= 0)
	{
		if(bit1 & byte)
			*lbuf++ = oncolor;
		else
			*lbuf++ = offcolor;

		if((bit1>>=1) == 0)
		{
			bit1 = 0x80;
			byte = *mbytes++;
		}
	}
}

static void _grc_mask2blit(UBYTE *mbytes, const unsigned mbpr, 
	Coor src_x, Coor src_y, 
	const Raster *dest, 
	Coor dest_x, Coor dest_y,
	Coor width, Coor height, 
	const Pixel oncolor, const Pixel offcolor)

/* Expand a memory buffer arranged as a bit-plane into a rectangular
   area of dest raster.  Used to implement graphics text and icons.
   1's in bit-plane are set to oncolor in dest.  
   0's in bit-plane are set to offcolor in dest. */
/* (Unclipped.) */

{
Pixel *lbuf;
Pixel sbuf[SBUF_SIZE/sizeof(Pixel)];
UBYTE bit1;

	lbuf = sbuf;
	if(width > Array_els(sbuf))
	{
		if((lbuf = pj_malloc(width*sizeof(Pixel))) == NULL)
			return;
	}

	bit1 = (0x80>>(src_x&7));
	mbytes += (src_x>>3) + src_y*mbpr;

	while(--height >= 0)
	{
		mask2line(mbytes,bit1,lbuf,width,oncolor,offcolor);
		mbytes += mbpr;
		PUT_HSEG(dest,lbuf,dest_x,dest_y++,width);
	}
	if(lbuf != sbuf)
		pj_free(lbuf);
}

static void grc_swaprect(Raster *ra,			 	 /* raster a */
			  LONG ax, LONG ay,  		 /* ra Minx and Miny */
			  Raster *rb,   		     /* raster b */
			  LONG bx, LONG by, 		 /* rb minx and miny */
			  LONG width, LONG height) /* blit size */  


/* unclipped swap rectangles in a and b this is NOT clipped */
{
UBYTE la[150], lb[150];
LONG wbx, wax;
LONG width_left;
LONG get_width;

	if(width <= sizeof(la))
	{
		while(height--)
		{
			GET_HSEG(ra,la,ax,ay,width);
			GET_HSEG(rb,lb,bx,by,width);
			PUT_HSEG(ra,lb,ax,ay++,width);
			PUT_HSEG(rb,la,bx,by++,width);
		}
		return;
	}

	while(height--)
	{
		wax = ax; 
		wbx = bx;
		width_left = width;
		get_width = sizeof(la);
		for(;;)
		{
			GET_HSEG(ra,la,wax,ay,get_width);
			GET_HSEG(rb,lb,wbx,by,get_width);
			PUT_HSEG(ra,lb,wax,ay,get_width);
			PUT_HSEG(rb,la,wbx,by,get_width);

			if((width_left -= sizeof(la)) <= 0)
				break;
			wax += get_width;
			wbx += get_width;
			if(width_left < get_width)
				get_width = width_left;
		}
		++ay;
		++by;
	}
	return;
}


static Errcode grc_tblitrect(const Raster *src,	/* source raster */
				  Coor src_x, Coor src_y,	/* source Minx and Miny */
				  const Raster *dest,		/* destination raster */
				  Coor dest_x, Coor dest_y, /* destination minx and miny */
				  Coor width, Coor height,  /* blit size */  
				  const Pixel tcolor)		/* transparent color */
/* Copys rectangle from src to dest except for transparent color in
   source. */
{
Pixel *source_buf, *dest_buf;
Pixel sbuf[SBUF_SIZE/sizeof(Pixel)];
Pixel ptcolor;

	ptcolor = tcolor;
	source_buf = sbuf;
	if(width > (Array_els(sbuf)/2))
	{
		if ((source_buf = pj_malloc((width+width)*sizeof(Pixel))) == NULL)
			return(Err_no_memory);
	}
	dest_buf = source_buf + width;
	while(height--)
	{
		GET_HSEG(src,source_buf,src_x,src_y++,width);
		GET_HSEG(dest,dest_buf,dest_x,dest_y,width);
		pj_tbli_line(source_buf, dest_buf, width, (void *)&ptcolor);
		PUT_HSEG(dest,dest_buf,dest_x,dest_y++,width);
	}
	if(source_buf != sbuf)
		pj_free(source_buf);
	return(Success);
}

static void xor_line(Pixel *source, Pixel *dest, Coor count)
/* (Private to grc_driver.) */
{
Pixel *maxdest;

	maxdest = dest + count;
	while(dest < maxdest)
		*dest++ ^= *source++;
}

static Errcode grc_xor_rast(Raster *source, Raster *dest)
/* Xor source raster into dest.  Assumes source and dest are same size */
{
Pixel *source_buf, *dest_buf;
Coor width, height;
Pixel sbuf[SBUF_SIZE/sizeof(Pixel)];
int y;

	width = dest->width;
	height = dest->height;
	y = 0;

	source_buf = sbuf;
	if(width > (Array_els(sbuf)/2))
	{
		if ((source_buf = pj_malloc((width+width)*sizeof(Pixel))) == NULL)
			return(Err_no_memory);
	}

	dest_buf = source_buf + width;
	while (--height >= 0)
	{
		GET_HSEG(source,source_buf,0,y,width);
		GET_HSEG(dest,dest_buf,0,y,width);
		xor_line(source_buf, dest_buf, width);
		PUT_HSEG(dest,dest_buf,0,y++,width);
	}
	if(source_buf != sbuf)
		pj_free(source_buf);
	return(Success);
}


/* this isn't static because bytemap lib uses it */

Errcode pj_grc_zoomblit( Raster *source,		/* source raster */
	           Coor src_x, Coor src_y,  /* source Minx and Miny */
	           Raster *dest,   		/* destination raster */
	           Coor dest_x, Coor dest_y,   /* destination minx and miny */
	           Ucoor width, Ucoor height,  /* destination blit size */  
	           LONG zoom_x, LONG zoom_y )  /* zoom scalers */
/* Move rectangular area of raster expanding pixels as you go. */
{
Pixel *hline; /* note not too big here will have to be re-done */
Coor swidth;
Coor maxy;
Coor nexty;
Pixel *testzx;
Pixel *zend;
Pixel *lend;
int rightpix;
Pixel sbuf[SBUF_SIZE/sizeof(Pixel)];

	if(!height)
		return(0);

	hline = sbuf;
	if(width > Array_els(sbuf))
	{
		if ((hline = pj_malloc(width*sizeof(Pixel))) == NULL)
			return(Err_no_memory);
	}
	swidth = width/zoom_x;

	if((rightpix = width%zoom_x) != 0)
		++swidth;
	else
		rightpix = zoom_x;

	maxy = dest_y + height; 
	nexty = dest_y;

	for(;;)
	{
		GET_HSEG(source,hline,src_x,src_y,swidth);
		++src_y;

		zend = &hline[width] - 1;
		lend = &hline[swidth] - 1;

		testzx = zend - rightpix;
		for(;;)
		{
			if(testzx < hline)
			{
				while(zend >= hline)
					*zend-- = *lend;
				break;
			}
			while(zend > testzx)
				*zend-- = *lend;
			--lend;
			testzx -= zoom_x;
		}
		if((nexty += zoom_y) >= maxy)
		{
			while(dest_y < maxy)
				PUT_HSEG(dest,hline,dest_x,dest_y++,width);
			goto OUT;
		}
		while(dest_y < nexty)
			PUT_HSEG(dest,hline,dest_x,dest_y++,width);
	}
OUT:
	if(hline != sbuf)
		pj_free(hline);
	return(Success);
}
#ifndef FLILIB_CODE
static void grc_diag_to_ptable( Raster *src, Pixel *dseg, Ucoor dsize,
			   			        Coor x0, Coor y0, Coor x1, Coor y1)

/* copy from an arbitrary line in src to a pixel array dsize long starting
   at dseg. This scales the pixels read from the source to the size of the
   destination buffer */
{
int incx, incy;
int dx, dy;
int dots;
int xerr, yerr;
Boolean didx;
Pixel spixel;

	if((dx = x1 - x0) < 0)
	{
		incx = -1;
	}
	else
	{
		incx = 1;
		dx = -dx;
	}
	if((dy = y1 - y0) < 0)
	{
		incy = -1;
	}
	else
	{
		incy = +1;
		dy = -dy;
	}
	dots = dsize;
	xerr = dots + (dx>>1);
	yerr = dots + (dy>>1);
	dx -= 1;
	dy -= 1;

	spixel = pj__get_dot(src,x0,y0);
	didx = FALSE;

	while (--dots >= 0)
	{
		*dseg++ = spixel; 	/* output one pixel */
		if((xerr += dx) <= 0)
		{
			didx = TRUE;
NEXTX:      
			x0 += incx;
			if ((xerr += dsize) <= 0)
				goto NEXTX;
		}
		if((yerr += dy) > 0)
		{
			if(!didx)
				continue;
		}
		else
		{
NEXTY:
			y0 += incy;
			if ((yerr += dsize) <= 0)
				goto NEXTY;
		}
		spixel = pj__get_dot(src,x0,y0);
		didx = FALSE;
	}
}
#endif /* FLILIB_CODE */

#ifdef SLUFFED
void grc_wseg(const Raster *v, const SHORT pix2, Coor x, Coor y, Coor count)
{
#define WPSZ 100
SHORT pbuf[WPSZ];
short lc;

while (count > 0)
	{
	lc = count;
	if (lc > WPSZ)
		lc = WPSZ;
	pj_stuff_words(pix2, pbuf, lc);
	PUT_HSEG(v, pbuf, x, y, lc+lc);
	count -= lc;
	}
#undef WPSZ
}
#endif /* SLUFFED */


void pj_grc_load_fullcalls(struct rastlib *lib)
/* Return pointer to generic display function jump-table */
{
#ifdef NEVER
	lib->put_dot = _xxx_put_dot;
	lib->get_dot = _xxx_get_dot;
#endif /* NEVER */

	pj_grc_load_dcompcalls(lib);

#ifdef IN_DCOMPCALLS

	lib->put_hseg = _grc_put_hseg;
	lib->put_rectpix = grc_put_rectpix;
	lib->set_hline = _grc_set_hline;
	lib->set_rect = _grc_set_rect;
	lib->set_rast = grc_set_rast;

	lib->unbrun_rect = grc_unbrun_rect;
	lib->unlccomp_rect = grc_unlccomp_rect;
	lib->unss2_rect = grc_unss2_rect;

	lib->uncc64 = grc_uncc64;
	lib->uncc256 = grc_uncc256;

#endif /* IN_DCOMPCALLS */

	pj_grc_load_compcalls(lib);

#ifdef IN_COMPCALLS
	lib->get_hseg = _grc_get_hseg;
	lib->get_rectpix = grc_get_rectpix;
#endif /* IN_COMPCALLS */

	pj_grc_load_commcalls(lib);

#ifdef IN_COMMONCALLS

	lib->cput_dot = _grc_cput_dot;
	lib->cget_dot = _grc_cget_dot;

	/* binary calls */

	lib->blitrect[RL_TO_SAME] = grc_blitrect;
	lib->blitrect[RL_TO_BYTEMAP] = grc_t_blitrect;
	lib->blitrect[RL_FROM_BYTEMAP] = grc_f_blitrect;
	lib->blitrect[RL_TO_OTHER] = grc_blitrect;
#endif /* IN_COMMONCALLS */

	lib->put_vseg = _grc_put_vseg;
	lib->get_vseg = _grc_get_vseg;	

	lib->set_vline = _grc_set_vline;

	lib->xor_rect = _grc_xor_rect;
	lib->mask1blit = (rl_type_mask1blit)_grc_mask1blit;
	lib->mask2blit = (rl_type_mask2blit)_grc_mask2blit;

	lib->swaprect[RL_TO_SAME] = grc_swaprect;
	lib->swaprect[RL_TO_BYTEMAP] = grc_swaprect;
	lib->swaprect[RL_FROM_BYTEMAP] = grc_swaprect;
	lib->swaprect[RL_TO_OTHER] = grc_swaprect;

	lib->tblitrect[RL_TO_SAME] = grc_tblitrect;
	lib->tblitrect[RL_TO_BYTEMAP] = grc_tblitrect;
	lib->tblitrect[RL_FROM_BYTEMAP] = grc_tblitrect;
	lib->tblitrect[RL_TO_OTHER] = grc_tblitrect;

	lib->xor_rast[RL_TO_SAME] = grc_xor_rast;
	lib->xor_rast[RL_TO_BYTEMAP] = grc_xor_rast;
	lib->xor_rast[RL_FROM_BYTEMAP] = grc_xor_rast;
	lib->xor_rast[RL_TO_OTHER] = grc_xor_rast;

	lib->zoomblit[RL_TO_SAME] = pj_grc_zoomblit;
	lib->zoomblit[RL_TO_BYTEMAP] = pj_grc_zoomblit;
	lib->zoomblit[RL_FROM_BYTEMAP] = pj_grc_zoomblit;
	lib->zoomblit[RL_TO_OTHER] = pj_grc_zoomblit;

#ifndef FLILIB_CODE
	lib->diag_to_ptable = grc_diag_to_ptable;
#endif /* FLILIB_CODE */
}
