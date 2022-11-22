
/* convex.c - polygon fill routine.  Tests first to see if poly is
   y convex (meaning any horizontal line drawn through it would 
   intersect only a single segment of polygon.)  If this is the case
   it uses a horizontal line oriented polygon fill contained here.
   Otherwise it passes the buck to fill_concave() */

#include "stdtypes.h"
#include "errcodes.h"
#include "syslib.h"
#include "sld.h"

static Errcode blast_hlines(
SHORT *ebuf1, 		/* 1st list of x points.  Ordered from lo y to high */
SHORT *ebuf2, 		/* other list of x points.  Ordered from high y to lo */
SHORT highy, 			/* y value of 1st hline */
SHORT count,			/* height (# of hlines to draw */
EFUNC hline,			/* function to take horizontal lines */
void *hldat)			/* and some data for that function */
{
register SHORT x1, x2;
Errcode err = Success;

ebuf2 += count;
while (--count >= 0)
	{
	x1 = *ebuf1++;
	x2 = *(--ebuf2);
	if (x1 > x2)
		err = (*hline)(highy++, x2, x1, hldat);
	else
		err = (*hline)(highy++, x1, x2, hldat);
	if (err != 0)
		break;
	}
return(err);
}

static SHORT * xdda_ebuf(SHORT *ebuf, SHORT x1, SHORT y1, SHORT x2, SHORT y2)
/* Put x coordinates of a line from (x1,y1) to (x2,y2) into ebuf.
   Only put one x coordinate for each y value.  Make sure jaggies
   match with pj_cline() while you're at it. */
{
SHORT *eb;
int ebinc;
register SHORT   duty_cycle;
register SHORT delta_x, delta_y;
register SHORT dots;

	if ((delta_y = y2-y1) < 0) 
		delta_y = -delta_y;
	if ((delta_x = x2-x1) < 0) 
	{
		delta_x = -delta_x;
		x1 = x2;
		ebinc = -1;
		eb = ebuf + delta_y;
	}
	else
	{
		eb = ebuf;
		ebinc =  1;
	}
	duty_cycle = (delta_x - delta_y)/2;
	if (delta_x >= delta_y)
	{
		*eb = x1;
		eb += ebinc;
		dots = ++delta_x;
		while (--dots > 0)
		{
			duty_cycle -= delta_y;
			x1 += 1;
			if (duty_cycle < 0)
			{
				duty_cycle += delta_x;	  /* update duty cycle */
				*eb = x1;
				eb += ebinc;
			}
		}
	}
	else
	{
		dots = ++delta_y;
		while (--dots >= 0)
		{
			*eb = x1;
			eb += ebinc;
			duty_cycle += delta_x;
			if (duty_cycle > 0)
			{
				duty_cycle -= delta_y;	  /* update duty cycle */
				x1 += 1;
			}
		}
		--delta_y;
	}
	return(ebuf+delta_y);
}


/***********
**
**	fill_ebuf	-   fill in edge buffer
**
************/
static SHORT *fill_ebuf(LLpoint *thread, SHORT count, SHORT *ebuf)
{
SHORT x, y, ox, oy;

ox = thread->x;
oy = thread->y;
while (--count >= 0)
   {
   thread = thread->next;
   x = thread->x;
   y = thread->y;
   if (y!=oy)
	  ebuf = xdda_ebuf(ebuf,ox,oy,x,y);
   ox = x;
   oy = y;
   }
return(ebuf+1);
}


Errcode fill_poly_inside(Poly *pl, EFUNC hline, void *hldat)
{
Errcode err;
register LLpoint *p;
register LLpoint *np;
LLpoint *peak;
LLpoint *valley;
register SHORT highy;
register SHORT i;
register SHORT pcount;
#define SMAX 128
SHORT short_thread1[SMAX], short_thread2[SMAX];	/* keep short ones on stack */
SHORT *thread1, *thread2;
SHORT ycount;

peak = p = pl->clipped_list;
i = pl->pt_count;
highy = p->y;
pcount = 0;
while (--i > 0)
	{
	p = p->next;
	if (p->y <= highy)
		{
		peak = p;
		highy = p->y;
		}
	}
p = peak;
np = p->next;
i = pl->pt_count;
while (--i >= 0)
	{
	if (np->y < p->y)
		{
		valley = p;
		p = np;
		np = np->next;
		while (--i >= 0)
			{
			if (np->y > p->y)
				{
				return(fill_concave(pl, hline, hldat));/* sorry it's concave */
				}
			p = np;
			np = np->next;		
			}
		ycount = valley->y - peak->y + 1;
		if (ycount > SMAX)
			{
			if ((thread1 = malloc((long)ycount*sizeof(SHORT)*2L)) == NULL)
				return(Err_no_memory);
			thread2 = thread1 + ycount;
			}
		else 
			{
			thread1 = short_thread1;
			thread2 = short_thread2;
			}
		fill_ebuf(peak, pcount, thread1);
		pcount = fill_ebuf(valley, pl->pt_count - pcount, thread2)
			- thread2;
		err = blast_hlines(thread1, thread2, highy, pcount, hline, hldat);
		if (ycount > SMAX)
			free(thread1);
		return(err);
		}
	pcount++;
	p = np;
	np = np->next;
	}	
return(Success);
}


