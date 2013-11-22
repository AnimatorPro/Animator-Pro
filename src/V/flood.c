
/* Flood.c - a little flood fill routine.  Nearly portable! A horizontal
   segment/FIFO oriented flood fill.
   */

#include "jimk.h"

static UBYTE fcolor;

#define FSZ 1024
#define FMOD 1023

struct fseg
	{
	WORD y;
	WORD left, right;
	};

static struct fseg *fsegments;
static int fread_pt, fwrite_pt, fcount;
static PLANEPTR visitbuf;
static int floodto;

static UBYTE lmasks[] = {0xff,0x7f,0x3f,0x1f,0x0f,0x07,0x03,0x01};
static UBYTE rmasks[] = {0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xff};
UBYTE bmasks[] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

static
visit_hline(y, x1, x2)
WORD y, x1, x2;
{
register PLANEPTR pt;
WORD xbyte;
WORD bcount;

xbyte = (x1>>3);
pt = visitbuf + y*Mask_line(XMAX) + xbyte;
if ((bcount = (x2>>3)-xbyte) == 0)
	{
	*pt |= (lmasks[x1&7] & rmasks[x2&7]);
	}
else
	{
	*pt++ |= lmasks[x1&7];
	while (--bcount > 0)
		*pt++ = 0xff;
	*pt |= rmasks[x2&7];
	}
}



static
expand_fseg(s)
struct fseg *s;
{
register int x;
int y;
PLANEPTR line;
register PLANEPTR pt;
int f2;

f2 = floodto;
/* go left */
line = uf.p + BPR*s->y;
x = s->left;
pt = line + x;
while (x > 0)
	{
	if ((*(--pt) != fcolor)^f2)
		break;
	--x;
	}
s->left = x;
x = s->right;
pt = line+x+1;
while (x < XMAX-1)
	{
	if (( *pt++ != fcolor)^f2)
		break;
	x++;
	}
s->right = x;
}

static
add_fseg(y, left, right)
WORD y, left, right;
{
register struct fseg *new;

new = fsegments + fwrite_pt;
fwrite_pt++;
fwrite_pt &= FMOD;
fcount++;
fcount &= FMOD;
new->y = y;
new->left = left;
new->right = right;
}


static
scan_seg(y, left, right)
WORD y, left, right;
{
register WORD fillit;
register PLANEPTR pt;
WORD ln;
register WORD x;
int f2;

fillit = 0;
f2 = floodto;
pt = uf.p + BPR*y + left;
for (x=left; x<=right; x++)
	{
	if ((*pt++ == fcolor)^f2)
		{
		if (!fillit)
			{
			fillit = 1;
			ln = x;
			}
		}
	else
		{
		if (fillit)
			{
			fillit = 0;
			add_fseg(y, ln, x-1);
			}
		}
	}
if (fillit)
	add_fseg(y, ln, x-1);
}

static fhx0, fhx1, fhy0, fhy1;

static nofunc(){}

static 
flood_rhline(y, x0, x1, color)
WORD y, x0, x1,color;
{
render_hline(y, x0, x1);
}

fill(x,y)
unsigned x, y;
{
fcolor = getd(uf.p,x,y);
floodto = 0;
some_flood(x,y);
}

flood(x,y,endcolor)
unsigned x, y, endcolor;
{
fcolor = endcolor;
floodto = 1;
some_flood(x,y);
}


static
some_flood(x,y)
unsigned x, y;
{
if ((fsegments = begmem(FSZ*sizeof(*fsegments))) == NULL)
	return;
if ((visitbuf = lbegmem(Mask_block((long)XMAX,(long)YMAX))) == NULL)
	{
	freemem(fsegments);
	return;
	}
switch (vs.draw_mode)
	{
	case 1:
	case 2:
		flood_visit(nofunc,x,y,vs.ccolor);
		render_xy(fhx0,fhy0,fhx1,fhy1);
		break;
	default:
		break;
	}
if (make_render_cashes())
	{
	flood_visit(flood_rhline,x,y,vs.ccolor);
	free_render_cashes();
	}
freemem(fsegments);
freemem(visitbuf);
}


/* XMAX*YMAX/8 */
#define M_SIZE 8000

static
flood_visit(hout,x,y,color)
Vector hout;
unsigned x, y, color;
{
int left, right;
struct fseg *next;

fread_pt = fwrite_pt = fcount = 0;
fhx0=fhx1=x;
fhy0=fhy1=y;
stuff_words(0, visitbuf, 
	M_SIZE/sizeof(WORD) );
add_fseg(y, x, x);
while (fcount > 0)
	{
	next = fsegments + fread_pt;
	fread_pt++;
	fread_pt&=FMOD;
	fcount -= 1;
	y = next->y;
	left = next->left;
	if (!(visitbuf[y*Mask_line(XMAX) + (left>>3)] & bmasks[left&7] ))
		{
		expand_fseg(next);
		left = next->left;
		right = next->right;
		visit_hline(y, left, right);
		(*hout)(y, left, right, color);
		if (y<fhy0)
			fhy0 = y;
		if (y>fhy1)
			fhy1 = y;
		if (left < fhx0)
			fhx0 = left;
		if (right > fhx1)
			fhx1 = right;
		if (y < YMAX-1)
			scan_seg(y+1, left, right);
		if (y > 0)
			scan_seg(y-1, left, right);
		}
	}
}



