
/* rotate.c - Stuff to implement cel/turn.  Real rotation work is
   done elsewhere in assembler.  This is an interface to the
   'persp' code also used by stretch and optics. */

#include "jimk.h"

/* a structure used to essentially hold food for diag_to_table */
struct rot_seg
	{
	Point s;
	WORD dxoff;	/* x offset of this line of dest */
	};
typedef struct rot_seg Rot_seg;

struct thread
	{
	WORD count;
	struct point dpoints[4];
	struct point spoints[4];
	};

struct min_max
	{
	WORD xmin, ymin;
	WORD xmax, ymax;
	WORD ymin_ix;
	WORD ymax_ix;
	WORD height;
	};
extern struct min_max minmax;
extern Vcel *ccc;

static Rot_seg *rs1, *rs2;

#ifdef CCODE
/* copy from a diagonal line in s to a horizontal line dsize long starting
   at 'dtable'. */
diag_to_table(s,sbpr,dtable,dsize,x0,y0,x1,y1)
register PLANEPTR s;
PLANEPTR dtable;
WORD sbpr;
register WORD dsize;
WORD x0,y0,x1,y1;
{
WORD incx, incy;
WORD dx, dy;
WORD dots;
WORD xerr, yerr;

s += x0;
s += (unsigned)y0*(unsigned)sbpr;
if ((dx = x1 - x0) < 0)
	{
	incx = -1;
	}
else
	{
	incx = 1;
	dx = -dx;
	}
if ((dy = y1 - y0) < 0)
	{
	incy = -sbpr;
	}
else
	{
	incy = sbpr;
	dy = -dy;
	}
dots = dsize;
xerr = dots + (dx>>1);
yerr = dots + (dy>>1);
dx -= 1;
dy -= 1;
while (--dots >= 0)
	{
	*dtable++ = *s;	/* move one pixel */
	if ((xerr += dx) <= 0)
		{
NEXTX:
		s += incx;
		if ((xerr += dsize) <= 0)
			goto NEXTX;
		}
	if ((yerr += dy) <= 0)
		{
NEXTY:
		s += incy;
		if ((yerr += dsize) <= 0)
			goto NEXTY;
		}
	}
}
#endif CCODE




static Point source_poly[4], dest_poly[4];
static struct thread thread1, thread2;
static struct min_max minmax, ominmax;

/* sq_poly - make up a 4 element polygon from a rectangle */
sq_poly(w, h, x, y, dest)
WORD w, h;
WORD x,y;
register WORD *dest;
{
w += x-1;
h += y-1;
*dest++ = x;
*dest++ = y;

*dest++ = x;
*dest++ = h;

*dest++ = w;
*dest++ = h;

*dest++ = w;
*dest++ = y;
}

/* find the bounding box of a polygon */
static
find_min_max(source)
register WORD *source;
{
register WORD a;
register WORD i;

copy_structure(&minmax, &ominmax, sizeof(ominmax));
minmax.xmin = minmax.xmax = *source++;
minmax.ymin = minmax.ymax = *source++;
minmax.ymin_ix = minmax.ymax_ix = 0;
for (i=1; i<4; i++)
	{
	a = *source++;
	if (a < minmax.xmin)
		minmax.xmin = a;
	if (a > minmax.xmax)
		minmax.xmax = a;
	a = *source++;
	if (a < minmax.ymin)
		{
		minmax.ymin = a;
		minmax.ymin_ix = i;
		}
	if (a > minmax.ymax)
		{
		minmax.ymax = a;
		minmax.ymax_ix = i;
		}
	}
minmax.height = minmax.ymax - minmax.ymin + 1;
}

static
find_thread(thread, dir)
register struct thread *thread;
WORD dir;
{
register WORD ix, oix;
register WORD count;
int first;

first = 1;
oix = ix = minmax.ymin_ix;
count = 0;
do
	{
	ix += dir;
	ix &= 3;
	if (first)
		{
		count = 0;
		if (dest_poly[oix].y != dest_poly[ix].y)
			{
			thread->dpoints[0] = dest_poly[oix];
			thread->spoints[0] = source_poly[oix];
			first = 0;
			}
		}
	count++;
	thread->dpoints[count] = dest_poly[ix];
	thread->spoints[count] = source_poly[ix];
	oix = ix;
	}
while (ix != minmax.ymax_ix);
thread->count = count;
}


/* convert a thread to a rot_seg list ... essentially the dest x position
	and the source x,y  that corresponds to a dest y position implecit
	in the index of the rot_seg list */

extern Vector cbvec;	/* 'blit' function */
extern Vector cmvec;	/* 'move' function */
extern render_blit();


static
tcel_to_screen(scel, dscreen, seg1, seg2, ylines, yoff, first)
Vcel *scel;
Vscreen *dscreen;
struct rot_seg *seg1, *seg2;
int ylines;
int yoff;
int first;
{
PLANEPTR dimage, uimage;
int x1, x2, swap;
struct rot_seg *swapr;
int wid;
PLANEPTR linebuf;
PLANEPTR d,u;
int dbpr;
Vector blitfunc;
int lbsize;
void *lastparam;

set_zero_clear();
lastparam = uf.p;
switch (first)
	{
	case 0:
		blitfunc = cmvec;
		break;
	case 1:
		blitfunc = cbvec;
		break;
	case 2:
		blitfunc = render_blit;
		lastparam = scel->cmap;
		break;
	}
d = dscreen->p;
u = uf.p;
lbsize = minmax.xmax - minmax.xmin + 1;
if ((linebuf = begmem(lbsize)) == NULL)
	return(0);
while (--ylines >= 0)
	{
	if (yoff >= 0 && yoff < YMAX)
		{
		x1 = seg1->dxoff;
		x2 = seg2->dxoff;
		if (x1 > x2)
			{
			swap = x1;
			x1 = x2;
			x2 = swap;
			swapr = seg1;
			seg1 = seg2;
			seg2 = swapr;
			}
		if (!first)	/* need to erase last version of this??? */
			{
			if (ominmax.xmin < x1)
				blit8(x1 - ominmax.xmin, 1,
					ominmax.xmin, yoff, u, BPR,
					ominmax.xmin, yoff, d, BPR);
			if (x2 < ominmax.xmax)
				blit8(ominmax.xmax-x2+1, 1,
					x2+1, yoff, u, BPR,
					x2+1, yoff, d, BPR);
			}
		wid = x2-x1+1;
		dto_table(scel->p,scel->bpr, linebuf,
			wid, seg1->s.x,seg1->s.y,seg2->s.x,seg2->s.y);
		(*blitfunc)(wid, 1, 0, 0, linebuf, BPR, x1, yoff, d, BPR,
			vs.inks[0], lastparam);
		}
	seg1++;
	seg2++;
	yoff++;
	}
freemem(linebuf);
return(1);
}

/* Make up a rot_seg from a thread.  A rot_seg's basically a list for 
	1 side of a convex polygon with 1 element for each scan-line of 
	the polygon.  This becomes food for my diagonal line to horizontal 
	line mapper. */
static
fill_sbuf(thread, seg)
struct thread *thread;
struct rot_seg *seg;
{
int tcount;
int ds, ddx, dsx, dsy;
int dxerr, sxerr, syerr;
int dx, sx, sy;
int incdx, incsx, incsy;
Point *dpt, *spt;
int dots;
int err;

tcount = thread->count;
dpt = thread->dpoints;
spt = thread->spoints;
/* copy 1st x dot into seg */
seg->dxoff = dpt->x;
seg->s.x = spt->x;
seg->s.y = spt->y;
seg++;
while (--tcount >= 0)
	{
	if ((dots = ds = (dpt+1)->y - dpt->y) == 0)  /* skip horizontal segments */
		{
		spt++;
		dpt++;
		continue;
		}
	dx = dpt->x;
	sx = spt->x;
	sy = spt->y;
	if ((ddx = (dpt+1)->x - dx) < 0)
		{
		incdx = -1;
		ddx = -ddx;
		}
	else
		incdx = 1;
	if ((dsx = (spt+1)->x - sx) < 0)
		{
		incsx = -1;
		dsx = -dsx;
		}
	else
		incsx = 1;
	if ((dsy = (spt+1)->y - sy) < 0)
		{
		incsy = -1;
		dsy = -dsy;
		}
	else
		incsy = 1;
	dxerr = ddx - (ds>>1);
	sxerr = dsx - (ds>>1);
	syerr = dsy - (ds>>1);
	while (--dots >= 0)
		{
		while (dxerr > 0)
			{
			dx += incdx;
			dxerr -= ds;
			}
		while (sxerr > 0)
			{
			sx += incsx;
			sxerr -= ds;
			}
		while (syerr > 0)
			{
			sy += incsy;
			syerr -= ds;
			}
		seg->dxoff = dx;
		seg->s.x = sx;
		seg->s.y = sy;
		seg++;
		dxerr += ddx;
		sxerr += dsx;
		syerr += dsy;
		}
	spt++;
	dpt++;
	}
}



static
free_threads()
{
gentle_freemem(rs1);
rs1 = NULL;
}

static
build_threads(b, dpoly)
Vcel *b;
Point *dpoly;
{
long rssize;

sq_poly(b->w, b->h, 0, 0, source_poly);
copy_structure(dpoly, dest_poly, sizeof(dest_poly) );
/* find bounding box of dest_poly */
find_min_max(dest_poly);

/* single pixel high dests do screw things up.  Actually the bug is
   in find_thread or fill_sbuf, but the quick fix is here*/
if (minmax.height <= 1)
	{
	return(1);
	}
/* try to get buffer to hold the "segment-list" that will eventually
   pass to fast raster-rotater */
rssize = 2 * (long)minmax.height * sizeof(Rot_seg);
if (rssize >= 60000L)
	{
	otoobig();
	return(0);
	}
if ((rs1 = (Rot_seg *)lbegmem(rssize)) == NULL)
	{
	return(0);
	}
rs2 = rs1+(minmax.height);
/* go do all the twisted calls to make up the segment list */
find_thread(&thread1, 1);
find_thread(&thread2, -1);
fill_sbuf(&thread1, rs1);
fill_sbuf(&thread2, rs2);
return(1);
}


static
raster_t(b, dpoly, first)
Vcel *b;
Point *dpoly;
int first;
{
unsigned char *bbuf;	/* byte-a-pixel line buffer */
WORD bbuf_length;
int ow;

if (!build_threads(b, dpoly))
	return(0);
/* single pixel high dests do screw things up.  Actually the bug is
   in find_thread or fill_sbuf, but the quick fix is here*/
if (minmax.height <= 1)
	{
	free_threads();
	return(1);
	}
if (!first)
	{
	ow = ominmax.xmax+1 - ominmax.xmin;

	if (ominmax.ymin < minmax.ymin)
		{
		blit8(ow, minmax.ymin - ominmax.ymin,
			ominmax.xmin, ominmax.ymin,  uf.p, BPR,
			ominmax.xmin, ominmax.ymin,  render_form->p, BPR);
		}
	}
tcel_to_screen(b, render_form, rs1, rs2, minmax.height, minmax.ymin, first);
if (!first)
	{
	if (ominmax.ymax > minmax.ymax)
		{
		blit8(ow, ominmax.ymax - minmax.ymax,
			ominmax.xmin, minmax.ymax+1,  uf.p, BPR,
			ominmax.xmin, minmax.ymax+1,  render_form->p, BPR);
		}
	}
free_threads();
return(1);
}

raster_transform(b, dpoly, first)
Vcel *b;
Point *dpoly;
int first;
{
int ret;

if (first == 2)
	{
	if (!make_brender_cashes())
		return(0);
	find_min_max(dpoly);
	render_xy(minmax.xmin,minmax.ymin,minmax.xmax,minmax.ymax);
#ifdef LATER
	render_full_screen();
#endif LATER
	}
ret =  raster_t(b, dpoly, first);
if (first == 2)
	free_brender_cashes();
return(ret);
}




static
in_dpoly()
{
int myy;
int swaper;
int x1, x2;

if (grid_x >= minmax.xmin && grid_x < minmax.xmax && 
	grid_y >= minmax.ymin && grid_y < minmax.ymax)
	{
	if (minmax.height <= 1)	/* yuck special case */
		return(1);
	myy = grid_y - minmax.ymin;
	x1 = (rs1+myy)->dxoff;
	x2 = (rs2+myy)->dxoff;
	if (x1 > x2)
		return(grid_x <= x1 && grid_x >= x2);
	else
		return(grid_x <= x2 && grid_x >= x1);
	}
else
	return(0);
}

static int theta;
extern int render_first;
static Point dpoly[4];
static int cenx, ceny;

static
rsee_tcel(c)
Vcel *c;
{
sq_poly(c->w, c->h, c->x, c->y, dpoly);
rotate_points(theta, cenx, ceny, dpoly, dpoly, 4);
raster_transform(c, dpoly, render_first);
zoom_it();
}


vrotate_cel()
{
int ox,oy;
int dx,dy, odx,ody;
int otheta;
int ltheta;
int ttheta;
int inside;
int ocelx, ocely;
int x0,y0;
int cccx1, cccy1;
char buf[8];

if (cel == NULL)
	return;
if (!make_ccc())
	return;
cccx1 = cel->x;
cccy1 = cel->y;
save_undo();
theta = 0;
render_first = 1;
for (;;)
	{
	otheta = theta;
	ocelx = ccc->x;
	ocely = ccc->y;
	cenx = (ccc->w>>1) + ccc->x;
	ceny = (ccc->h>>1) + ccc->y;
	rsee_tcel(ccc);
	marqi_poly(dpoly, 4);
	render_first = 0;
	wait_click();
	if (PJSTDN)
		{
		x0 = grid_x;
		y0 = grid_y;
		if (build_threads(ccc, dpoly))
			{
			inside = in_dpoly();
			free_threads();
			}
		else
			inside = 1;
		ltheta = -arctan(grid_x - cenx, grid_y - ceny);
		for (;;)
			{
			if (inside)
				{
				ccc->x = ocelx + grid_x - x0;
				ccc->y = ocely + grid_y - y0;
				cenx = (ccc->w>>1) + ccc->x;
				ceny = (ccc->h>>1) + ccc->y;
				}
			else
				{
				ttheta = -arctan(grid_x - cenx, grid_y - ceny);
				theta += ttheta-ltheta;
				if (theta < 0)
					theta += TWOPI;
				if (theta >= TWOPI)
					theta -= TWOPI;
				ltheta = ttheta;
				}
			rsee_tcel(ccc);
			sprintf(buf, "%4d", sscale_by(theta,360,TWOPI));
			ltop_text(buf);
			wait_input();
			if (PJSTDN)
				{
				break;
				}
			if (RJSTDN || key_hit)
				{
				theta = otheta;
				ccc->x = ocelx;
				ccc->y = ocely;
				break;
				}
			}
		}
	else
		{
		finish_stretch_rot(rsee_tcel);
		break;
		}
	}
cel->x = cccx1;
cel->y = cccy1;
}



