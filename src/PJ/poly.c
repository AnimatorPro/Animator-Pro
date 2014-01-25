
#include <limits.h>
#include "errcodes.h"
#include "imath.h"
#include "memory.h"
#include "poly.h"
#include "rectang.h"

void reverse_poly(Poly *p)
{
LLpoint *olist,  *newlist, *next;
int pcount;

pcount = p->pt_count;
olist = p->clipped_list;	/* 1st point will stay the same... */
newlist = NULL;
while (--pcount >= 0)
	{
	next = olist->next;
	olist->next = newlist;
	newlist = olist;
	olist = next;
	}
p->clipped_list->next = newlist;	/* put in the ring link */
}

Errcode poly_to_vertices(Poly *poly, Boolean closed, 
	Short_xyz **pvertices)
/* Convert a Polygon in circular linked list form to one that's an
 * array of x/y/z values.   If poly is closed, duplicate the first point
 * as the last point */
{
Short_xyz *v;
LLpoint *pts = poly->clipped_list;
int pcount = poly->pt_count+closed;

if ((*pvertices = v = pj_malloc(pcount*sizeof(*v))) == NULL)
	return(Err_no_memory);
while (--pcount >= 0)
	{
	v->x = pts->x;
	v->y = pts->y;
	v->z = pts->z;
	pts = pts->next;
	v += 1;
	}
return(Success);
}


void poly_ave_3d(Poly  *p, Short_xyz *v)
{
long lx,ly,lz;
int count = p->pt_count;
int i = count;
LLpoint *s = p->clipped_list;

lx = ly = lz = 0;
while (--i >= 0)
	{
	lx += s->x;
	ly += s->y;
	lz += s->z;
	s = s->next;
	}
v->x = lx/count;
v->y = ly/count;
v->z = lz/count;
}

int calc_zpoly( register Short_xyz *s,
			           register Short_xy *d,
			           int count, int xcen, int ycen, int ground_z)

/* Transform 3-d pointlist into 2-d pointlist doing perspective
   calculations the cheap way */
{
int x, y, z;
#define TOOBIG 10000


	while (--count >= 0)
	{
		z = s->z + ground_z;
		if (z < 1)
			return(Err_overflow);
		x = d->x = sscale_by(s->x-xcen, ground_z, z) + xcen;
		y = d->y = sscale_by(s->y-ycen, ground_z, z) + ycen;
		if (x < -TOOBIG || x > TOOBIG || y < -TOOBIG || y > TOOBIG)
			return(Err_overflow);
		d += 1;
		s += 1;
	}
	return(0);
#undef TOOBIG
}

void poly_to_3d(Poly *sp, Short_xyz *d)
/* convert a poly with linked list to an array of points */
{
int count = sp->pt_count;
LLpoint *pt = sp->clipped_list;

while (--count>=0)
	{
	*d++ = *((Short_xyz *)&pt->x);
	pt = pt->next;
	}
}
void poly_bounds(Poly *p, Cliprect *cr)
{
int count = p->pt_count;
LLpoint *pt = p->clipped_list;

	cr->MaxY = cr->MaxX = SHRT_MIN;
	cr->x = cr->y = SHRT_MAX;
	while (--count>=0)
	{
		if(pt->x < cr->x)
			cr->x = pt->x;
		if(pt->x > cr->MaxX)
			cr->MaxX = pt->x;
		if(pt->y < cr->y)
			cr->y = pt->y;
		if(pt->y > cr->MaxY)
			cr->MaxY = pt->y;
		pt = pt->next;
	}
	/* cause cliprects are 1 beyond */
	++cr->MaxX;
	++cr->MaxY;
}

