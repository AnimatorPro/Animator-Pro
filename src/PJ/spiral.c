/* spiral.c - help draw a spiral */

#include "jimk.h"
#include "errcodes.h"
#include "fli.h"
#include "marqi.h"
#include "pentools.h"
#include "poly.h"

static int get_rub_spiral(void);

static int make_spiral_poly(int x0, int y0, int rad, int itheta, long ttheta)
{
int i;
register int ppoints;
register LLpoint *next;

free_polypoints(&working_poly);
ppoints = 32*(ttheta/TWOPI);
ppoints = intabs(ppoints);
ppoints += 32;
working_poly.pt_count = ppoints+1;
for (i=0; i<=ppoints; i++)
	{
	if ((next = begmem(sizeof(*next))) == NULL)
		{
		free_polypoints(&working_poly);
		return(0);
		}
	next->next = working_poly.clipped_list;
	working_poly.clipped_list = next;
	polar((int)((long)itheta + 
		((long)ttheta*i+ppoints/2)/ppoints),
		(int)(((long)rad*i+ppoints/2)/ppoints),
		&next->x);
	next->x += x0;
	next->y += y0;
	next->z = 0;
	}
poly_last_point(&working_poly)->next = working_poly.clipped_list;
return(1);
}

Errcode spiral_tool(Pentool *pt, Wndo *w)
{
	Errcode err;
	(void)pt;
	(void)w;

	if (!pti_input())
		return(Success);
	if((err = get_rub_spiral()) < 0)
		free_polypoints(&working_poly);
	else
	{
		err = finish_polyt(FALSE,FALSE);
		save_redo_spiral();
	}
	return(err);
}

static int spi_rad;
static long spi_ttheta;

static int get_rub_spiral(void)
{
Errcode err;
int itheta;
int t0,t1,dt;
Short_xy xys[2];
Marqihdr mh;

	save_undo();
	/* 1st get initial angle... */
	if((err = get_rub_line(xys)) < 0)
		return(err);
	vinit_marqihdr(&mh,0,1);

	t0 = -arctan(xys[1].x-xys[0].x, xys[1].y-xys[0].y);
	itheta = t0+TWOPI/4;
	spi_ttheta = 0;
	for (;;)
	{
		spi_rad = calc_distance(xys[0].x,xys[0].y,icb.mx,icb.my);
		t1 = -arctan(icb.mx-xys[0].x, icb.my-xys[0].y);
		dt = t1-t0;
		if (dt > TWOPI/2)
			dt = dt-TWOPI;
		if (dt < -TWOPI/2)
			dt = dt+TWOPI;
		spi_ttheta += dt;
		t0 = t1;
		soft_top_textf("!%6d", "top_deg", spi_ttheta*360/TWOPI);
		if (!make_spiral_poly(xys[0].x,xys[0].y,spi_rad,itheta,spi_ttheta))
			return(0);
		marqi_polydots(&mh,&working_poly);
		wait_input(MBPEN|MBRIGHT|MMOVE);
		undo_polydots(&mh,&working_poly);
		if (JSTHIT(MBPEN|MBRIGHT))
			break;
	}
	cleanup_toptext();
	if(JSTHIT(MBPEN))
		return(0);
	return(Err_abort);
}

