
#include <stdio.h>
#include "evga.h"


Pixel _evga_get_dot(EvgaRast *r,Coor x, Coor y);
void _evga_put_dot(EvgaRast *r,Pixel color, Coor x, Coor y);
void evga_set_colors(EvgaRast *r,LONG start, LONG count, UBYTE *cmap);
void _evga_d_hline(EvgaRast *r,Pixel color, Coor x, Coor y, Ucoor width);
void _evga_x_hline(EvgaRast *r,Pixel color, Coor x, Coor y, Ucoor width);
void _evga_put_hseg(EvgaRast *r,Pixel *buf, Coor x, Coor y, Ucoor width);
void _evga_get_hseg(EvgaRast *r,Pixel *buf, Coor x, Coor y, Ucoor width);
void evga_cset_colors64(EvgaRast *r,void *ucbuf);
void evga_cset_colors256(EvgaRast *r,void *ucbuf);
extern void evga_wait_vblank(EvgaRast *r);


void _evga_mask1(UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
			   long address, long bank, long bpr,
			   Ucoor width, Ucoor height,
			   Pixel oncolor );

void evga_unss2(EvgaRast *r, void *cbuf, Ytable *ytable, Coor xoff, Coor yoff,
	Ucoor width);

void evga_ss2_cleanup(EvgaRast *v, void *ucbuf, 
	LONG xorg, LONG yorg, ULONG width)
/* set the last pixel in the line if needed */
{
Coor y;
SHORT lp_count;
SHORT opcount;
int psize;
union {SHORT *w; UBYTE *ub; BYTE *b;} wpt;
Coor lastx;

	lastx = xorg + width - 1;
	wpt.w = ucbuf;
	lp_count = *wpt.w++;
	y = yorg;
	goto LPACK;

SKIPLINES:

	y -= opcount;

LPACK:	/* do next line */
	if ((opcount = *wpt.w++) >= 0)
		goto DO_SS2OPS;

	if( ((USHORT)opcount) & 0x4000) /* skip lines */
		goto SKIPLINES;

	PUT_DOT(v,(UBYTE)opcount,lastx,y); /* put dot at eol with low byte */
	if((opcount = *wpt.w++) == 0)
	{
		++y;
		if (--lp_count > 0)
			goto LPACK;
		return;
	}
DO_SS2OPS:

PPACK:				/* do next packet */
	wpt.ub+=1;
	psize = *wpt.b++;
	if ((psize += psize) >= 0)
	{

		wpt.ub += psize;
		if (--opcount != 0)
			goto PPACK;
		++y;
		if (--lp_count > 0)
			goto LPACK;
	}
	else
	{
		psize = -psize;
		++wpt.w;
		if (--opcount != 0)
			goto PPACK;
		++y;
		if (--lp_count > 0)
			goto LPACK;
	}
}



static void evga_unss2_rect(EvgaRast *r,void *ucbuf, LONG pixsize,
				   Coor x,Coor y,Ucoor width,Ucoor height)
/* Word at a time delta decompression routine */
{
evga_unss2(r, ucbuf, r->hw.em.ytable, x, y, width);
if (width&1)
	evga_ss2_cleanup(r,ucbuf, x, y, width);
}


/* $$$$ Research if need to worry about split here. */

static void _evga_mask1_blit(UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
	EvgaRast *r, Coor rx, Coor ry, Ucoor width, Ucoor height, Pixel oncolor)
/* break up into blits that are within a bank */
{
Ytable *yt, *oyt;
Coor lh;
SHORT bank;
Coor split_at;
Coor w1;

yt = r->hw.em.ytable;
yt += ry;
bank = yt->bank;
if (bank == yt[height].bank)	/* no problem, all in one bank */
	_evga_mask1(mbytes,mbpr,mx,my, yt->address+rx,bank,r->hw.em.bpr,
		width,height,oncolor);
else
	{
	for (;;)
		{
		oyt = yt;
		lh = 0;
		while (lh < height)
			{
			if ((split_at = yt->split_at) != 0)
				break;
			yt++;
			lh++;
			}
		if (lh > 0)
			{
			_evga_mask1(mbytes, mbpr, mx, my, 
				oyt->address+rx, oyt->bank, r->hw.em.bpr,
				width, lh, oncolor);
			ry += lh;
			my += lh;
			if ((height -= lh) <= 0)
				break;
			}
		if (split_at)
			{
			if (rx >= split_at)
				{
				_evga_mask1(mbytes, mbpr, mx, my, 
					yt->address+rx-0x10000, yt->bank+1, r->hw.em.bpr,
					width, 1, oncolor);
				}
			else if (rx + width <= split_at)
				{
				_evga_mask1(mbytes, mbpr, mx, my, 
					yt->address+rx, yt->bank, r->hw.em.bpr,
					width, 1, oncolor);
				}
			else	/* split right in middle */
				{
				w1 = split_at-rx;
				_evga_mask1(mbytes, mbpr, mx, my,
					yt->address+rx, yt->bank, r->hw.em.bpr,
					w1, 1, oncolor);
				_evga_mask1(mbytes, mbpr, mx+w1, my,
					0xa0000, yt->bank+1, r->hw.em.bpr,
					width-w1, 1, oncolor);
				}
			ry++;
			my++;
			yt++;
			if ((height -= 1) <= 0)
				break;
			}
		}
	}
}

static void evga_xor_rect(EvgaRast *v, Pixel color, 
	Coor x, Coor y, Ucoor width, Ucoor height)
/* xor a rectangle with a solid color */
{
while (--height)
	{
	_evga_x_hline(v, color, x, y++, width);
	}
}



void *evga_get_rlib(void)
{
static Rastlib evga_raster_library;
static got_lib = 0;

	if (!got_lib)
	{
		evga_raster_library.put_dot = _evga_put_dot;
		evga_raster_library.get_dot = _evga_get_dot;
		evga_raster_library.set_hline = _evga_d_hline;
		evga_raster_library.put_hseg = _evga_put_hseg;
		evga_raster_library.get_hseg = _evga_get_hseg;
		evga_raster_library.mask1blit = _evga_mask1_blit;
		evga_raster_library.unss2_rect = evga_unss2_rect;
		evga_raster_library.xor_rect = evga_xor_rect;

		evga_raster_library.wait_vsync = evga_wait_vblank, /* wait_vsync */
		evga_raster_library.set_colors = evga_set_colors;
		evga_raster_library.uncc64 = evga_cset_colors64;
		evga_raster_library.uncc256 = evga_cset_colors256;
		got_lib = 1;
	}
	return(&evga_raster_library);
}


