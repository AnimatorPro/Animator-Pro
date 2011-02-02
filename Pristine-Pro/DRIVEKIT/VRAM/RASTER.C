#include "vram.h"

extern void vram_wait_vsync(VramRast *r);
Pixel _vram_get_dot(VramRast *r,Coor x, Coor y);
void _vram_put_dot(VramRast *r,Pixel color, Coor x, Coor y);
void vram_set_colors(VramRast *r,LONG start, LONG count, UBYTE *cmap);
void _vram_d_hline(VramRast *r,Pixel color, Coor x, Coor y, Ucoor width);
void _vram_x_hline(VramRast *r,Pixel color, Coor x, Coor y, Ucoor width);
void _vram_put_hseg(VramRast *r,Pixel *buf, Coor x, Coor y, Ucoor width);
void _vram_get_hseg(VramRast *r,Pixel *buf, Coor x, Coor y, Ucoor width);
void vram_uncc64(VramRast *r,void *ucbuf);
void vram_uncc256(VramRast *r,void *ucbuf);


void _vram_mask1(UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
			   long address, long bank, long bpr,
			   Ucoor width, Ucoor height,
			   Pixel oncolor );

void vram_unss2(VramRast *r, void *cbuf, Ytable *ytable, Coor xoff, Coor yoff,
	Ucoor width);

static void vram_ss2_cleanup(VramRast *v, void *ucbuf, 
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

	CPUT_DOT(v,(UBYTE)opcount,lastx,y); /* put dot at eol with low byte */
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



static void vram_unss2_rect(VramRast *r,void *ucbuf, LONG pixsize,
				   Coor x,Coor y,Ucoor width,Ucoor height)
{
vram_unss2(r, ucbuf, r->hw.vm.ytable, x, y, width);
if (width&1)	/* if odd width, must jump through hoops`*/
	{
	vram_ss2_cleanup(r,ucbuf, x, y, width);
	}
}


static void _vram_mask1_blit(UBYTE *mbytes, Coor mbpr, Coor mx, Coor my,
	VramRast *r, Coor rx, Coor ry, Ucoor width, Ucoor height, Pixel oncolor)
/* break up into blits that are within a bank */
{
Ytable *yt, *oyt;
Coor lh;
SHORT bank;
Coor split_at;
Coor w1;

yt = r->hw.vm.ytable;
yt += ry;
bank = yt->bank;
if (bank == yt[height].bank)	/* no problem, all in one bank */
	_vram_mask1(mbytes,mbpr,mx,my, yt->address+rx,bank,r->hw.vm.bpr,
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
			_vram_mask1(mbytes, mbpr, mx, my, 
				oyt->address+rx, oyt->bank, r->hw.vm.bpr,
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
				_vram_mask1(mbytes, mbpr, mx, my, 
					yt->address+rx-0x10000, yt->bank+1, r->hw.vm.bpr,
					width, 1, oncolor);
				}
			else if (rx + width <= split_at)
				{
				_vram_mask1(mbytes, mbpr, mx, my, 
					yt->address+rx, yt->bank, r->hw.vm.bpr,
					width, 1, oncolor);
				}
			else	/* split right in middle */
				{
				w1 = split_at-rx;
				_vram_mask1(mbytes, mbpr, mx, my,
					yt->address+rx, yt->bank, r->hw.vm.bpr,
					w1, 1, oncolor);
				_vram_mask1(mbytes, mbpr, mx+w1, my,
					0xa0000, yt->bank+1, r->hw.vm.bpr,
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

#ifdef SLUFFED
static void vram_xor_rect(VramRast *v, Pixel color, 
	Coor x, Coor y, Coor width, Coor height)
{
while (--height>=0)
	{
	_vram_x_hline(v, color, x, y++, width);
	}
}
#endif /* SLUFFED */

void *vram_get_rlib(void)
{
static Rastlib vram_raster_library;
static got_lib = 0;

	if (!got_lib)
	{
		vram_raster_library.put_dot = _vram_put_dot;
		vram_raster_library.get_dot = _vram_get_dot;
		vram_raster_library.set_hline = _vram_d_hline;
		vram_raster_library.put_hseg = _vram_put_hseg;
		vram_raster_library.get_hseg = _vram_get_hseg;
		vram_raster_library.mask1blit = _vram_mask1_blit;
		vram_raster_library.unss2_rect = vram_unss2_rect;
#ifdef NOT_YET
		vram_raster_library.xor_rect = vram_xor_rect;

		vram_raster_library.uncc64 = vram_uncc64;
		vram_raster_library.uncc256 = vram_uncc256;
#endif
		vram_raster_library.wait_vsync = vram_wait_vsync, /* wait_vsync */
		vram_raster_library.set_colors = vram_set_colors;
		got_lib = 1;
	}
	return(&vram_raster_library);
}


