/* cel.c - routines to manage a Vcel (which is basically a rectangular
   byte-a-pixel image buffer with an associated color map and position
   on the screen.)  */

#include "jimk.h"

free_cel(c)
Vcel *c;
{
if (c != NULL)
	{
	gentle_freemem(c->cmap);
	gentle_freemem(c->p);
	freemem(c);
	}
}

Vcel *
alloc_cel(w,h,x,y)
unsigned w,h,x,y;
{
Vcel *c;

if ((c = begmem(sizeof(*c))) == NULL)
	return(NULL);
if ((c->p = lbegmem(Raster_block((long)w, (long)h))) == NULL)
	{
	freemem(c);
	return(NULL);
	}
if ((c->cmap = begmem(COLORS*3)) == NULL)
	{
	freemem(c->p);
	freemem(c);
	return(NULL);
	}
c->bpr = Raster_line(w);
c->w = w;
c->h = h;
c->x = x;
c->y = y;
return(c);
}

Vcel *
alloc_bcel(w,h,x,y)
unsigned w,h,x,y;
{
Vcel *c;

if ((c = alloc_cel(Mask_line(w), h, x, y)) != NULL)
	{
	c->w = w;
	}
return(c);
}

see_a_cel(cl)
register Vcel *cl;
{
blit8(cl->w, cl->h, 0, 0, cl->p, cl->bpr, 
	cl->x, cl->y, vf.p, vf.bpr);
}


tile_cel(c)
Vcel *c;
{
int ox,oy,x,y;
int w, h;
int ix,iy;

if (c == NULL )
	return;
if ((w = c->w) < 1)
	return;
if ((h = c->h) < 1)
	return;
ox = x = c->x;
oy = y = c->y;
while (x > 0)
	x -= w;
while (y > 0)
	y -= h;
for (ix = x; ix < XMAX; ix+=w)
	for (iy = y; iy < YMAX; iy += h)
		{
		c->x = ix;
		c->y = iy;
		see_a_cel(c);
		}
c->x = ox;
c->y = oy;
}

move_cel(c)
Vcel *c;
{
int dx,dy;
int mx,my;
char buf[40];

if (c == NULL)
	return;
mx = uzx;
my = uzy;
dx = c->x - uzx;
dy = c->y - uzy;
for (;;)
	{
	c->x = uzx + dx;
	c->y = uzy + dy;
	tile_s_cel(c);
	sprintf(buf, " %d %d (%d %d)", c->x, c->y, uzx - mx, uzy - my);
	stext(buf, 0, 0, sblack, swhite);
	wait_input();
	if (PJSTDN||RJSTDN||key_hit)
		break;
	}
if (!PJSTDN)
	{
	c->x = mx + dx;
	c->y = my + dy;
	}
tile_s_cel(c);
}

tile_bit_cel(c)
Vcel *c;
{
int x, y, w, h;
int ix, iy;

x = c->x;
y = c->y;
w = c->w;
h = c->h;
if (w < 1 || h < 1)
	return;
while (x > 0)
	x -= w;
while (y > 0)
	y -= h;
for (ix = x; ix < XMAX; ix+=w)
	for (iy = y; iy < YMAX; iy += h)
		{
		a2blit(w,h,0,0,c->p,c->bpr,ix,iy,vf.p,vf.bpr,1, 0);
		}
}




flip_cel(c)
Vcel *c;
{
UBYTE *p1, *p2;
int i;
int bpr, h;

h = c->h;
bpr = c->bpr;
p1 = c->p;
p2 = long_to_pt(pt_to_long(p1)+(long)bpr*(h-1) );
h /= 2;
while (--h >= 0)
	{
	exchange_bytes(p1,p2,bpr);
	p1 = norm_pointer(p1+bpr);
	p2 = long_to_pt(pt_to_long(p2) - bpr);
	}
}

