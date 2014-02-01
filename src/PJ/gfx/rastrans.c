/* rastrans.c - Stuff to implement cel/turn.  Real rotation work is
   done elsewhere in assembler.  This is an interface to the
   'persp' code also used by stretch and optics. */

#define RASTRANS_C
#include "errcodes.h"
#include "gfx.h"
#include "memory.h"
#include "rastrans.h"

/* a structure used to essentially hold food for diag_to_table */

typedef struct rot_seg {
	Short_xy s;
	SHORT dxoff;	/* x offset of this line of dest */
} Rot_seg;

typedef struct thread {
	SHORT count;
	Short_xy dpoints[4];
	Short_xy spoints[4];
} Thread;

typedef struct rxfdata {
	Short_xy source_poly[4];
	Boolean moved;
	Rot_seg *rs1, *rs2;
	Rcel *src_cel;
	Thread thread1;
	Thread thread2;
} Rxfdata;


void init_xformspec(Xformspec *xf)
{
	clear_mem(xf,sizeof(Xformspec));
}
void load_rect_minmax(Rectangle *rect,Xformspec *xf)
{
	rect_tofrect(rect,(Fullrect *)&(xf->mmax.FRECTSTART));
	xf->mmax.ymin_ix = 0;
	xf->mmax.ymax_ix = 3;
}
void load_poly_minmax(Xformspec *xf)
/* find the bounding box of a flicel polygon and load min max data */
{
register SHORT *source = (SHORT *)&(xf->bpoly[0]);
register SHORT a;
register SHORT i;

	xf->mmax.x = xf->mmax.MaxX = *source++;
	xf->mmax.y = xf->mmax.MaxY= *source++;
	xf->mmax.ymin_ix = xf->mmax.ymax_ix = 0;
	for (i=1; i<4; i++)
	{
		a = *source++;
		if (a < xf->mmax.x)
			xf->mmax.x = a;
		if (a > xf->mmax.MaxX)
			xf->mmax.MaxX = a;
		a = *source++;
		if (a < xf->mmax.y)
		{
			xf->mmax.y = a;
			xf->mmax.ymin_ix = i;
		}
		if (a > xf->mmax.MaxY)
		{
			xf->mmax.MaxY = a;
			xf->mmax.ymax_ix = i;
		}
	}
	/* note that maxs are one pixel greater than size */
	++xf->mmax.MaxY;
	++xf->mmax.MaxX;
	xf->mmax.height = xf->mmax.MaxY - xf->mmax.y;
	xf->mmax.width = xf->mmax.MaxX - xf->mmax.x;
}


static void find_thread(register Thread *thread,
						Xformspec *xf, Rxfdata *rxd,
						SHORT dir)
{
register SHORT ix, oix;
register SHORT count;
Boolean first;

	first = TRUE;
	oix = ix = xf->mmax.ymin_ix;
	count = 0;
	do
	{
		ix += dir;
		ix &= 3;
		if (first)
		{
			count = 0;
			if (xf->bpoly[oix].y != xf->bpoly[ix].y)
			{
				thread->dpoints[0] = xf->bpoly[oix];
				thread->spoints[0] = rxd->source_poly[oix];
				first = FALSE;
			}
		}
		count++;
		thread->dpoints[count] = xf->bpoly[ix];
		thread->spoints[count] = rxd->source_poly[ix];
		oix = ix;
	}
	while (ix != xf->mmax.ymax_ix);

	thread->count = count;
}

static void fill_sbuf(Thread *thread,struct rot_seg *seg)

/* Make up a rot_seg from a thread.  A rot_seg's basically a list for 
	1 side of a convex polygon with 1 element for each scan-line of 
	the polygon.  This becomes food for my diagonal line to horizontal 
	line mapper. */
{
int tcount;
int ds, ddx, dsx, dsy;
int dxerr, sxerr, syerr;
int dx, sx, sy;
int incdx, incsx, incsy;
Short_xy *dpt, *spt;
int dots;

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
		/* skip horizontal segments */
		if ((dots = ds = (dpt+1)->y - dpt->y) == 0) 
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
			++seg;
			dxerr += ddx;
			sxerr += dsx;
			syerr += dsy;
		}
		spt++;
		dpt++;
	}
}
static void free_threads(Rxfdata *rxd)
{
	pj_freez(&(rxd->rs1));
}
static Errcode build_threads(Xformspec *xf, Rxfdata *rxd)
{
long rssize;
Rcel *b;

	b = rxd->src_cel;
	rxd->rs1 = NULL;
	sq_poly(b->width, b->height, 0, 0, rxd->source_poly);
	load_poly_minmax(xf); /* find bounding box of dpoly */

	/* single pixel high dests do screw things up.  Actually the bug is
	   in find_thread or fill_sbuf, but the quick fix is here*/
	if (xf->mmax.height <= 1)
		return(Success);

	/* try to get buffer to hold the "segment-list" that will eventually
	   pass to fast raster-rotater */
	rssize = 2 * (long)xf->mmax.height * sizeof(Rot_seg);
	if(rssize >= 60000L)
		return(Err_too_big);

	if ((rxd->rs1 = (Rot_seg *)pj_malloc(rssize)) == NULL)
		return(Err_no_memory);

	rxd->rs2 = rxd->rs1+(xf->mmax.height);
	/* go do all the twisted calls to make up the segment list */
	find_thread(&rxd->thread1, xf, rxd, 1);
	find_thread(&rxd->thread2, xf, rxd, -1);
	fill_sbuf(&rxd->thread1, rxd->rs1);
	fill_sbuf(&rxd->thread2, rxd->rs2);
	return(Success);
}

Errcode raster_transform(Rcel *src_cel,Rcel *dscreen,Xformspec *xf,
						 Errcode (*putline)(void *plinedat, Pixel *line,
						 					Coor x, Coor y, Ucoor width), 
						 void *plinedat,
						 Boolean erase_last,

						 /* these are only needed if erase_last is true */

						 void (*undraw_line)(Coor x, Coor y, Ucoor width,
						 					 void *edat), 
						 void (*undraw_rect)(Coor x, Coor y,
						 					 Ucoor width, Ucoor height,
											 void *edat), 
						 void *edat)
{
Errcode err;
Rxfdata rxd;
SHORT height,yoff;
int x1, xmax, swap; /* x1 is stzrt of line xmax is one pixel beyond line */
Rot_seg *swapr, *seg1, *seg2;
int wid;
Pixel *srcline;

	/* single pixel high dests do screw things up.  Actually the bug is
	   in find_thread or fill_sbuf, but the quick fix is here*/

	if(xf->mmax.height <= 1)
		return(Success); /* successful ?? */

	rxd.src_cel = src_cel;
	if((err = build_threads(xf, &rxd)) < 0)
		return(err);

	if(NULL == (srcline = pj_malloc(xf->mmax.width*sizeof(Pixel))))
	{
		err = Err_no_memory;
		goto error;
	}

	if(erase_last && ((height = xf->mmax.y - xf->ommax.y) > 0))
	{
		(*undraw_rect)(xf->ommax.x,xf->ommax.y,
				  	   xf->ommax.width,Min(height,xf->ommax.height),edat);
	}

	seg1 = rxd.rs1;
	seg2 = rxd.rs2;
	yoff = xf->mmax.y;
	height = xf->mmax.height;

	while (--height >= 0)
	{
		/* note that yoff < 0 is really huge as unsigned */

		if(((USHORT)yoff) < dscreen->height)
		{
			x1 = seg1->dxoff;
			xmax = seg2->dxoff;
			if (x1 > xmax)
			{
				swap = x1;
				x1 = xmax;
				xmax = swap;
				swapr = seg1;
				seg1 = seg2;
				seg2 = swapr;
			}
			++xmax; /* one bigger to conform with MaxX in ommax & mmax */

			if(erase_last && yoff >= xf->ommax.y && yoff < xf->ommax.MaxY)
			{
				if (x1 > xf->ommax.x)
				{
					if((wid = x1 - xf->ommax.x) > xf->ommax.width)
						wid = xf->ommax.width;
					(*undraw_line)(xf->ommax.x,yoff,wid,edat);
				}
				if (xmax < xf->ommax.MaxX)
				{
					if((wid = xf->ommax.MaxX-xmax) >= xf->ommax.width)
						(*undraw_line)(xf->ommax.x,yoff,xf->ommax.width,edat);
					else
						(*undraw_line)(xmax,yoff,wid,edat);
				}
			}
			wid = xmax-x1;

			pj_diag_to_ptable(src_cel, srcline,
						   wid, seg1->s.x,seg1->s.y,seg2->s.x,seg2->s.y);

			if((err = (*putline)(plinedat,srcline,x1,yoff,wid)) < Success)
				goto error;
		}
		++seg1;
		++seg2;
		++yoff;
	}

	if(erase_last && ((height = xf->ommax.MaxY - xf->mmax.MaxY) > 0))
	{
		if(height >= xf->ommax.height)
		{
			(*undraw_rect)(xf->ommax.x,xf->ommax.y,
						   xf->ommax.width,xf->ommax.height,edat);
		}
		else
		{
			(*undraw_rect)(xf->ommax.x,xf->mmax.MaxY,
						   xf->ommax.width,height,edat);
		}
	}
	err = Success;

error:
	pj_free(srcline);
	free_threads(&rxd);
	return(err);
}

Boolean isin_bpoly(Xformspec *xf,Rcel *src_cel,SHORT x,SHORT y)
{
Boolean ret;
Rxfdata rxd;
SHORT myy;
SHORT x1, x2;

	rxd.src_cel = src_cel;
	if(build_threads(xf, &rxd) < Success)
		return(TRUE);

	if (x >= xf->mmax.x && x < xf->mmax.MaxX && 
		y >= xf->mmax.y && y < xf->mmax.MaxY)
	{
		if (xf->mmax.height <= 1)	/* yuck special case */
		{
			ret = TRUE;
		}
		else
		{
			myy = y - xf->mmax.y;
			x1 = (rxd.rs1+myy)->dxoff;
			x2 = (rxd.rs2+myy)->dxoff;
			if (x1 > x2)
				ret = (x <= x1 && x >= x2);
			else
				ret = (x <= x2 && x >= x1);
		}
	}
	else 
		ret = FALSE;
	free_threads(&rxd);
	return(ret);
}

