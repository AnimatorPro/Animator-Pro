#include <string.h>
#include "errcodes.h"
#include "imath.h"
#include "memory.h"
#include "poly.h"
#include "tween.h"

/* extra precision for tween */
#define TSHIFT 4

void init_tween_state(Tween_state *s)
{
clear_struct(s);
init_list(&s->links);
}

void trash_tween_state(Tween_state *state)
{
free_polypoints(&state->p0);
free_polypoints(&state->p1);
state->p0.polymagic = state->p1.polymagic = POLYMAGIC;
free_dl_list(&state->links);
init_tween_state(state);
}

Boolean tween_has_data(Tween_state *ts)
{
	return((ts->p0.polymagic==POLYMAGIC) && (ts->p1.polymagic==POLYMAGIC));
}

static void start_to_ends(Dlheader *list)
/* reverse start and end of each "tween_link" node in list */
{
Tween_link *link, *next;
int swap;

for (link = (Tween_link *)(list->head);
    NULL != (next = (Tween_link *)(link->node.next));
	link = next)
	{
	swap = link->start;
	link->start = link->end;
	link->end = swap;
	}
}

void tween_state_swap_ends(Tween_state *ts)
/* swap start and end polys */
{
Poly swap;
Poly *p0 = &ts->p0, *p1 = &ts->p1;

swap = *p0;
*p0 = *p1;
*p1 = swap;
start_to_ends(&ts->links);
isort_list(&ts->links, tween_cmp_link);
}


int tween_cmp_link(Tween_link *a, Tween_link *b)
/* compare two Tween_link's by start and end */
{
int dif;

dif = a->start - b->start;
if (dif == 0)
	dif = a->end - b->end;
return(dif);
}


static Errcode add_link(Dlheader *link, int start, int end, Tween_link **pnew)
/* Allocate, initialize and add to list a new link */
{
Tween_link *newl;
Errcode err;

if ((err = news(newl)) < Success)
	return(err);
newl->start = start;
newl->end = end;
insert_compare(&newl->node, link, tween_cmp_link);
*pnew = newl;
return(Success);
}


Errcode tween_add_a_link(Tween_state *ts, int startix, int endix
, Boolean closed, Tween_link **pnewl)
/* This will if necessary rotate the vertices of a closed polygon so
 * that the first points correspond to the first links.  
 * Then it adds the link.
 */
{
	Errcode err;
	int link_count;

	/* first links re-arranges clipped lists so that
	   it's always 0,0 */
	link_count = listlen(&ts->links);
	if (link_count == 0)
		{
		if (closed)
			{
			ts->p0.clipped_list = slist_el(ts->p0.clipped_list, startix);
			ts->p1.clipped_list = slist_el(ts->p1.clipped_list, endix);
			if ((err = add_link(&ts->links, 0, 0, 
				pnewl)) < Success)
				goto OUT;
			goto OUT;
			}
		else
			{
			if ((err = add_link(&ts->links, 0, 0, 
				pnewl)) < Success)
				goto OUT;
			/* fall through to 2-n point case */
			}
		}
	if ((err = add_link(&ts->links, startix, endix, 
		pnewl)) < Success)
		{
		goto OUT;
		}
OUT:
	return err;
}

static void
sample_vertex(Short_xyz *vtx, int vcount, int vsign,
		Short_xyz *delta_array, int scale)
/* Put position along path corresponding to scale into delta_array */
{
SHORT samp0, little_scale;
Short_xyz *samp_val0, *samp_val1;
SHORT samples, t0samp;

samples = vcount - 1;
if (samples < 1)
	{
	delta_array->x = delta_array->y = delta_array->z = 0;
	return;
	}
if (scale == SCALE_ONE)
	{
	if (vsign >= 0)
		samp_val0 = vtx+samples;
	else
		samp_val0 = vtx-samples;

	delta_array->x = samp_val0->x - vtx->x;
	delta_array->y = samp_val0->y - vtx->y;
	delta_array->z = samp_val0->z - vtx->z;
	}
else if  (scale == 0)
	{
	delta_array->x = delta_array->y = delta_array->z = 0;
	}
else
	{
	samp0 = (long)scale * samples / SCALE_ONE;
	t0samp = (long)samp0 * SCALE_ONE / samples;
	little_scale = (scale - t0samp) * samples;
	if (vsign >= 0)
		{
		samp_val0 = vtx+samp0;
		samp_val1 = vtx+samp0+1;
		}
	else
		{
		samp_val0 = vtx-samp0;
		samp_val1 = vtx-samp0-1;
		}
#define SCALE(a,b,l) (((a)*(l)+(b)*(SCALE_ONE-(l))+SCALE_ONE/2)/SCALE_ONE)
	delta_array->x = SCALE(samp_val1->x, samp_val0->x, little_scale) - vtx->x;
	delta_array->y = SCALE(samp_val1->y, samp_val0->y, little_scale) - vtx->y;
	delta_array->z = SCALE(samp_val1->z, samp_val0->z, little_scale) - vtx->z;
#undef SCALE
	}
}

Errcode calc_path_pos( Poly *poly,
						   Short_xyz *delta_array,
						   int scale, Boolean closed)
/* Put position along poly corresponding to scale into delta_array */
{
Short_xyz *vtx;
Errcode err;

if ((err = poly_to_vertices(poly, closed, &vtx)) >= Success)
	{
	sample_vertex(vtx,poly->pt_count+closed,1,delta_array,scale);
	pj_free(vtx);
	}
return(err);
}






static int lcm_zero_ok(int a, int b)
{
if (a == 0)
	return(b);
if (b == 0)
	return(a);
return(ilcm(a,b));
}

static void init_tw_list(Tw_tlist *twl)
{
clear_struct(twl);
init_list(&twl->list);
}


void trash_tw_list(Tw_tlist *twl)
{
pj_gentle_free(twl->spts);
pj_gentle_free(twl->dpts);
pj_gentle_free(twl->ipts);
free_dl_list(&twl->list);
init_tw_list(twl);
}

static Errcode new_tw_thread(Tw_tlist *tout,
	int start0, int end0,
	int start1, int end1)
{
Tw_thread *newt;
Errcode err;
int max;
int cm;

if ((err = news(newt)) < Success)
	return(err);
newt->source = tout->spts + start0;
newt->dest = tout->dpts + end0;
newt->scount = start1-start0;
newt->dinc = 1;
if ((newt->dcount = end1-end0) < 0)
	{
	newt->dcount = -newt->dcount;
	newt->dinc = -1;
	}
/* Figure out how many vertices will be in the intermediate shapes generated
 * by the tween.  This will either be the least common multiple of the
 * # of points in the source and the destination, or if that would be a
 * big number (greater than 21) then the maximum of the # of points in the
 * source and the destination. */
max = intmax(newt->scount, newt->dcount);
if ((cm = lcm_zero_ok(newt->scount, newt->dcount)) <= Success)
	return(cm);
if (cm <= 21)
	newt->icount = cm;
else
	newt->icount = max;
add_tail(&tout->list, &newt->node);
tout->icount += newt->icount;
return(Success);
}

static void find_inter_points(Tw_tlist *tls)
/* Calculate the endpoints of each thread within the output tween array */
{
Tw_thread *link, *next;
int ioff = 0;

for (link = (Tw_thread *)(tls->list.head);
	(next = (Tw_thread *)(link->node.next)) != NULL;
	link = next)
	{
	link->inter = tls->ipts+ioff;
	ioff += link->icount;
	}
}

static void lshift_vertices(Short_xyz *v, int count, int shift)
/* Scale up the xyz values of all vertices by a power of 2 */
{
while (--count >= 0)
	{
	v->x <<= shift;
	v->y <<= shift;
	v->z <<= shift;
	v += 1;
	}
}

static void rshift_vertices(Short_xyz *v, int count, int shift)
/* Scale up the xyz values of all vertices by a power of 2 */
{
while (--count >= 0)
	{
	v->x >>= shift;
	v->y >>= shift;
	v->z >>= shift;
	v += 1;
	}
}


Errcode ts_to_tw_list(Tween_state *vin, Boolean closed, Tw_tlist *tout)
/* Given a Tween_state - that is the beginning and end polygons and
 * a list of which points in the beginning poly are connected to which
 * points in the end,  generate a tween-list, which is an intermediate
 * form containing the array of points that will hold the result of
 * our tween calculations and some other stuff. */
{
Tween_link *link, *next;
int tct,sct,dct;
Errcode err;
int i;

init_tw_list(tout);
if ((err = poly_to_vertices(&vin->p0, closed, &tout->spts)) < Success)
	goto ERROR;
if ((err = poly_to_vertices(&vin->p1, closed, &tout->dpts)) < Success)
	goto ERROR;
tout->scount = vin->p0.pt_count;
tout->dcount = vin->p1.pt_count;
lshift_vertices(tout->spts, tout->scount+closed, TSHIFT);
lshift_vertices(tout->dpts, tout->dcount+closed, TSHIFT);
sct = vin->p0.pt_count + closed - 1;	/* Open polys have 1 less point that's
										 * considered during tween calcs. */
dct = vin->p1.pt_count + closed - 1;
tct = listlen(&vin->links);
if (tct <= 1)
	{
	if ((err = new_tw_thread(tout, 0, 0, sct, dct)) < Success)
		goto ERROR;
	}
else
	{
	link = (Tween_link *)(vin->links.head);
	next = (Tween_link *)(link->node.next);
	i = tct-1;
	while (--i >= 0)
		{
		if ((err = new_tw_thread(tout, link->start, link->end,
			next->start, next->end)) < Success)
			goto ERROR;
		link = next;
		next = (Tween_link *)(link->node.next);
		}
	if ((err = new_tw_thread(tout, link->start, link->end, 
		sct, dct)) < Success)
		goto ERROR;
	}
if (!closed)
	tout->icount += 1;
if ((tout->ipts = pj_malloc((tout->icount)*(long)sizeof(Short_xyz))) == NULL)
	{
	err = Err_no_memory;
	goto ERROR;
	}
find_inter_points(tout);
return(Success);
ERROR:
trash_tw_list(tout);
return(err);
}


#ifdef DEBUG
print_tw_threads(Tw_tlist *tls)
{
Tw_thread *link, *next;

for (link = (Tw_thread *)(tls->list.head);
	(next = (Tw_thread *)(link->node.next)) != NULL;
	link = next)
	{
	boxf(
		"s-%d: (%d %d %d) (%d %d %d) %s d-%d: (%d %d %d) (%d %d %d) %s dinc %d"
		"  i-%d:",
		link->scount, 
		link->source[0].x, link->source[0].y, link->source[0].z,
		link->source[1].x, link->source[1].y, link->source[1].z,
		(link->scount > 2 ? "..." : ""),
		link->dcount,
		link->dest[0].x, link->dest[0].y, link->dest[0].z,
		link->dest[1].x, link->dest[1].y, link->dest[1].z,
		(link->dcount > 2 ? "..." : ""),
		link->dinc,
		link->icount);
	}
}
#endif

	/* calculate a intermediate between two values based on scale. 
	 * Also scale down the result to normal numbers. */
#define TMUL_SHIFT(x0,x1,scale) (((x0)+itmult((x1)-(x0),scale))>>TSHIFT)

static Short_xyz *calc_tw_thread(
	Short_xyz *t0,  int t0_count, int t0_inc,
	Short_xyz *t1,  int t1_count, int t1_inc,
	Short_xyz *inter, int icount,
	int scale)
/* Do tween calculations on a single thread - that is the segment between
 * one link and another (or the whole poly if there's no links */
{
int tween_scale;
Short_xyz da0, da1;
int i;

#ifdef SOON
if (t0_count == t1_count)
	{
	while (--t0_count >= 0)
		{
		inter->x = TMUL_SHIFT(t0->x, t1->x, scale);
		inter->y = TMUL_SHIFT(t0->y, t1->y, scale);
		inter->z = TMUL_SHIFT(t0->z, t1->z, scale);
		inter += 1;
		t0 += t0_inc;
		t1 += t1_inc;
		}
	}
else
#endif /* SOON */
	{
	for (i=0; i<icount; i++)
		{
		tween_scale = pj_uscale_by(SCALE_ONE, i, icount);
		sample_vertex(t0,t0_count,t0_inc,&da0,tween_scale);
		/* path position is movement offset so add in 1st point position */
		da0.x += t0->x;
		da0.y += t0->y;
		da0.z += t0->z;
		sample_vertex(t1,t1_count,t1_inc,&da1,tween_scale);
		da1.x += t1->x;
		da1.y += t1->y;
		da1.z += t1->z;
		inter->x = TMUL_SHIFT(da0.x,da1.x,scale);
		inter->y = TMUL_SHIFT(da0.y,da1.y,scale);
		inter->z = TMUL_SHIFT(da0.z,da1.z,scale);
		inter += 1;
		}
	}
return(inter);
}

static void calc_lastp(Tw_tlist *tls, int scale)
/* Calculate the tween position of last point in an open poly tween. */
{
Short_xyz *lp0, *lp1;
Short_xyz *stopper = tls->ipts + tls->icount - 1;

lp0 = tls->spts + tls->scount - 1;
lp1 = tls->dpts + tls->dcount - 1;
stopper->x = TMUL_SHIFT(lp0->x, lp1->x, scale);
stopper->y = TMUL_SHIFT(lp0->y, lp1->y, scale);
stopper->z = TMUL_SHIFT(lp0->z, lp1->z, scale);
}

static void calc_tw_list(Tw_tlist *tls, Boolean closed, int scale)
{
Tw_thread *link, *next;

for (link = (Tw_thread *)(tls->list.head);
	(next = (Tw_thread *)(link->node.next)) != NULL;
	link = next)
	{
	calc_tw_thread(
		link->source, link->scount+1, 1,
		link->dest, link->dcount+1, link->dinc,
		link->inter, link->icount,
		scale );
	}
if (!closed)
	{
	calc_lastp(tls, scale);
	}
}


void calc_tween_points(Tw_tlist *tl, Boolean closed, int scale, 
	Short_xyz **ppts, int *pcount)
/* This returns an array of short_xyz points *pcount long in *ppts
 * which contains the inbetween polygon for the integer scaling
 * factor scale.  *ppts is not allocated here and must not be freed!
 *
 * This routine largely serves to handle the extreme cases for
 * scale == 0 and scale == SCALE_ONE which are not handled accurately
 * by the lower level calc_tw_list().
 */
{
int count;

*ppts = tl->ipts;
if (scale == 0)
	{
	count = tl->scount;
	memcpy(tl->ipts, tl->spts, count * sizeof(*(tl->ipts)) );
	rshift_vertices(tl->ipts, count, TSHIFT);
	}
else if (scale == SCALE_ONE)
	{
	count = tl->dcount;
	memcpy(tl->ipts, tl->dpts, count * sizeof(*(tl->ipts)) );
	rshift_vertices(tl->ipts, count, TSHIFT);
	}
else
	{
	calc_tw_list(tl, closed, scale);
	count = tl->icount;
	}
*pcount = count;
}

