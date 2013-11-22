
/* scanedge.c - a routine to draw a filled polygon with arbatrary
   edge crossings.  Allocates a bit-plane big enough to hold
   rasterized polygon, and then does and xorline/scanedge routine
   that's similar to what drives the Amiga's blitter.  */

#include "jimk.h"
#include "flicmenu.h"
#include "poly.h"
#include "scanedge.str"

extern sdot();

#define UPDIR 1
#define DOWNDIR 0



static UBYTE *on_off_buf;

int pxmin, pxmax, pymin, pymax;

find_pminmax(poly)
register Poly *poly;
{
register LLpoint *pointpt;
int i;

pointpt = poly->clipped_list;
pxmin = pxmax = pointpt->x;
pymin = pymax = pointpt->y;
pointpt = pointpt->next;

i = poly->pt_count;
while (--i > 0)
   {
   if (pxmin > pointpt->x) pxmin = pointpt->x;
   if (pxmax < pointpt->x) pxmax = pointpt->x;
   if (pymin > pointpt->y) pymin = pointpt->y;
   if (pymax < pointpt->y) pymax = pointpt->y;
   pointpt = pointpt->next;
   }
}

fill_concave(poly)
register Poly *poly;
{
register LLpoint *pointpt;
register WORD x,y;
register WORD ox,oy;
WORD lastdir;
WORD i;
WORD bpr;
WORD width, height;
long size;
WORD opw;
WORD ocolor;


find_pminmax(poly);
if (pymin==pymax)  /*can't cope with trivial case*/
	{
	render_opoly(poly);
	return(1);
	}

width = pxmax - pxmin + 1;
height = pymax - pymin + 1;
bpr = Mask_line(width);
size = ((long)bpr*height+1)&0xfffffe;
if (size > 64000)
	{
	continu_line(scanedge_101);
	return(0);
	}
if ((on_off_buf = lbegmem(size)) == NULL)	
	return(0);
zero_lots(on_off_buf, size);

pointpt = poly->clipped_list;
x = pointpt->x;
y = pointpt->y;

do
	{
	pointpt = pointpt->next;
	ox = pointpt->x;
	oy = pointpt->y;
	}
while (oy == y);

if (oy>y)
	lastdir = UPDIR;
else
	lastdir = DOWNDIR;

i = poly->pt_count;
while (--i >= 0)
   {
   pointpt = pointpt->next;
   x = pointpt->x;
   y = pointpt->y;
   if (y!=oy)
	  {
	  y_xor_line(bpr,ox-pxmin,oy-pymin,x-pxmin,y-pymin);
	  if (y>oy)
		 if (lastdir == UPDIR)
			xor_pt(bpr,ox-pxmin,oy-pymin);
		 else
			lastdir = UPDIR;
	  else
		 if (lastdir == DOWNDIR)
			xor_pt(bpr,ox-pxmin,oy-pymin);
		 else
			lastdir = DOWNDIR;
	  }
   ox = x;
   oy = y;
   }

/*run on off on it*/
on_off(bpr, width, height);
render_bitmap_blit(width, height, 0, 0, on_off_buf, bpr, 
	pxmin, pymin);
freemem(on_off_buf);
opw = vs.pen_width;
vs.pen_width = 0;
render_opoly(poly);
vs.pen_width = opw;
return(1);
}



static
on_off(bpr, width, height)
WORD bpr;
WORD width;
WORD height;
{
register UBYTE *imagept = on_off_buf;
register UBYTE *linept;
register UBYTE rot;
register WORD j;

while (--height >= 0)
	{
	linept = imagept;
	j = width;
start_off:
	while ( !*linept)
		{
		linept++;
		if ( (j-=8) <= 0)
			goto next_line;
		}
	rot = 0x80;
	while (!(rot & *linept))
		{
		if ( --j <= 0)
			goto next_line;
		rot >>=1;
		} /*scan til first on*/
	skip_first_on:
	if ( --j <= 0)
		goto next_line;
	rot >>=1;
start_on:
	for (;;)
		{
		if (rot == 0)
			{
			linept++;
			break;
			}
		if (! (rot & *linept) )  /*advance over blank bits*/
			*linept |= rot;
		else		 /*if found end segment in word*/
			goto end_in_word;
		if ( --j <= 0)
			goto next_line;
		rot >>=1;
		}
	while ( !*linept)
		{
		*linept = 0xff;
		linept++;
		if ( (j-=8) <= 0)
			goto next_line;
		}
	rot = 0x80;
end_in_word:
	for (;;) /*look for new segment in word */
		{
		if ( (rot & *linept) )
			{
			for (;;)
				{
				if ( --j <= 0)
				  goto next_line;
				rot >>= 1;
				if (rot == 0)
				  {
				  linept++;
				  goto start_off;
				  }
				if (rot & *linept)
				  goto skip_first_on;
				}
			}
		*linept |= rot;
		if ( --j <= 0)
			goto next_line;
		rot >>= 1;
		}
	next_line:
	imagept += bpr;
	}
}


static
xor_pt(bpr,x,y)
WORD bpr;
register WORD x;
WORD y;
{
register UBYTE rot;

rot = ((unsigned)0x80) >> (x&7);
on_off_buf[ bpr*y + (x>>3) ] ^= rot;
}


static
y_xor_line(bpr, x1, y1, x2, y2)
WORD bpr;
int x1,y1,x2,y2;
{
register UBYTE *imagept = on_off_buf;
WORD height;
register UBYTE rot;
register WORD   duty_cycle;
register WORD   delta_x, delta_y;
register WORD dots;
int swap;

if (x1 > x2)
	{
	swap = x1;
	x1 = x2;
	x2 = swap;
	swap = y1;
	y1 = y2;
	y2 = swap;
	}
delta_y = y2-y1;
delta_x = x2-x1;
rot = ((unsigned)0x80) >> (x1&7);
imagept += bpr*y1 + (x1>>3);


if (delta_y < 0) 
	{
	delta_y = -delta_y;
	bpr = -bpr;
	}
duty_cycle = (delta_x - delta_y)/2;
*(imagept) ^= rot;
if (delta_x >= delta_y)
	goto horizontal;
dots = delta_y;
while (--dots >= 0)
	{
	*(imagept) ^= rot;
	duty_cycle += delta_x;	  /* update duty cycle */
	imagept += bpr;
	if (duty_cycle > 0)
		{
		duty_cycle -= delta_y;
		rot >>= 1;
		if (rot == 0)
			{
			imagept++;
			rot = 0x80;
			}
		}
	}
return;

horizontal:
dots = delta_x;
while (--dots >= 0)
	{
	duty_cycle -= delta_y;	  /* update duty cycle */
	if (duty_cycle < 0)
		{
		*(imagept) ^= rot;
		duty_cycle += delta_x;
		imagept += bpr;
		}
	rot >>= 1;
	if (rot == 0)
		{
		imagept++;
		rot = 0x80;
		}
	}
return;
}



 
