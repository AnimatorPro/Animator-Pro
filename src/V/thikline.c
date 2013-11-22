
/* thikline. - one of the more horrifying modules at the time of writing.
   The idea is simply to draw a line wider than one pixel thick.  I used
   to do this by 'dragging a brush', but this gets less and less efficient
   as the brush size gets larger.  Instead we make up a 4 sided polygon 
   and polyfill it.  Figuring out exactly the values for these endpoints is
   the fun part.  I never could get the arithmetic to round correctly in
   some of the smaller cases (especially 4 and 8), so I'm using a table
   instead (ptps[]).  You figure out what angle the line is and use it as
   an index into this table to get the offsets of 2 points in the 4-gon
   from a vertex.  After that it's not so bad... */

#include <stdio.h>
#include "jimk.h"
#include "poly.h"


#define PDIV 8		/* How many sides in circular brush */
#define PDMK (PDIV-1)
#define PINC (TWOPI/PDIV)

extern render_dot(), render_brush();

#ifdef SLUFFED
static struct point ptp[PDIV];
#endif SLUFFED


static struct point ptp1[] = {
{0,0,}, {0,0,}, {0,0,}, {-1,0,}, 
{-1,0,}, {-1,-1,}, {0,-1,}, {0,-1,}, 
};

static struct point ptp2[] = {
{1,0,}, {1,0,}, {0,1,}, {0,1,}, 
{-1,0,}, {-1,0,}, {0,-1,}, {0,-1,}, 
};

/* A trouble case */
static struct point ptp3[] = {
{1,0,}, {0,1,}, {-1,1,}, {-2,0,}, 
{-2,-1,}, {-1,-2,}, {0,-2,}, {1,-1,}, 
};

static struct point ptp4[] = {
{2,0,}, {1,1,}, {0,2,}, {-1,1,}, 
{-2,0,}, {-1,-1,}, {0,-2,}, {1,-1,}, 
};

static struct point ptp5[] = {
{2,0,}, {1,1,}, {0,2,}, {-2,1,}, 
{-3,0,}, {-2,-2,}, {0,-3,}, {1,-2,}, 
};

static struct point ptp6[] = {
{3,0,}, {2,2,}, {0,3,}, {-2,2,}, 
{-3,0,}, {-2,-2,}, {0,-3,}, {2,-2,}, 
};

/* trouble case */
static struct point ptp7[] = {
{3,0,}, {2,1,}, {0,3,}, {-2,2,}, 
{-4,0,}, {-3,-2,}, {0,-4,}, {1,-3,}, 
};

static struct point ptp8[] = {
{4,0,}, {3,3,}, {0,4,}, {-3,3,}, 
{-4,0,}, {-3,-3,}, {0,-4,}, {3,-3,}, 
};

/* too thin in diagonals originally */
static struct point ptp9[] = {
{4,0,}, {3,2,}, {0,4,}, {-3,3,}, 
{-5,0,}, {-4,-3,}, {0,-5,}, {2,-4,}, 
};

/* too thick in diagonals originally */
static struct point ptp10[] = {
{5,0,}, {3,4,}, {0,5,}, {-4,3,}, 
{-5,0,}, {-3,-4,}, {0,-5,}, {4,-3,}, 
};

static struct point *ptps[] = {
NULL,ptp1,ptp2,ptp3,
ptp4,ptp5,ptp6,ptp7,
ptp8,ptp9,ptp10,
};

static struct llpoint rlp[4] =
	{
		{rlp+1,0,0,0,},
		{rlp+2,0,0,0,},
		{rlp+3,0,0,0,},
		{rlp+0,0,0,0,},
	};

static Poly rl_poly =
	{
	4, rlp, 1, 0,
	};

/* Draw a possibly thick line.  One end is capped with a round brush */
render_line(x,y,xx,yy)
int x, y, xx, yy;
{
int dx, dy;
int xoff, yoff;
int theta;
int ex, ey;	/* edge x and y */
int i, ix;
struct point *ptp;

if (vs.pen_width == 0)
	{
	cline(x, y, xx, yy, render_dot);
	return;
	}
ptp = ptps[vs.pen_width];
/* if thicker than 1 pixel go make up a polygon for fat line */
dx = xx-x;
dy = yy-y;
/* theta goes to 1 of 16 angle values */
theta = ((512-arctan(dx,dy) + PINC/2)&0x3ff)/PINC;
ix = (theta&PDMK);
xoff = ptp[ix].x;
yoff = ptp[ix].y;
rlp[0].x = xoff+x;
rlp[0].y = yoff+y;
rlp[3].x = xoff+xx;
rlp[3].y = yoff+yy;
ix += PDIV/2;
ix &= PDMK;
xoff = ptp[ix].x;
yoff = ptp[ix].y;
rlp[1].x = xoff+x;
rlp[1].y = yoff+y;
rlp[2].x = xoff+xx;
rlp[2].y = yoff+yy;
fill_concave(&rl_poly);
render_brush(xx,yy);
}

/* Draw a possibly thick line.  Both ends are capped with a round brush */
render_1_line(x,y,xx,yy)
WORD x,y,xx,yy;
{
render_brush(x,y);
render_line(x,y,xx,yy);
}
