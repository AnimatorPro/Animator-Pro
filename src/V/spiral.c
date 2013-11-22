
/* spiral.c - help draw a spiral */

#include "jimk.h"
#include "poly.h"
#include "fli.h"
#include "spiral.str"

extern WORD x_0,y_0,x_1,y_1;

extern Poly working_poly;
extern struct llpoint *poly_last_point();

/* some functions for line drawing */
extern int copydot(),marqidot(),sdot(), xdot();
extern a1bdot(), rbdot(), rbbrush();

extern int pxmin, pxmax, pymin, pymax;

static
make_spiral_poly(x0,y0,rad,itheta,ttheta)
int x0,y0,rad;
int itheta;
long ttheta;
{
int i;
register int ppoints;
register LLpoint *next;

poly_nopoints(&working_poly);
ppoints = 32*(ttheta/TWOPI);
ppoints = intabs(ppoints);
ppoints += 32;
working_poly.pt_count = ppoints+1;
for (i=0; i<=ppoints; i++)
	{
	if ((next = begmem(sizeof(*next))) == NULL)
		{
		poly_nopoints(&working_poly);
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
	}
poly_last_point(&working_poly)->next = working_poly.clipped_list;
working_poly.closed = 0;
return(1);
}


spiral_tool()
{
brushcursor = dot_pens[vs.pen_width];
if (!pti_input())
	return;
if (rub_spiral())
	{
	finish_polyt(0);
	}
else
	poly_nopoints(&working_poly);
}

static int spi_rad;
static long spi_ttheta;

static
rub_spiral()
{
int itheta;
int t0,t1,dt;
int cenx, ceny;
int lx,ly;
char buf[40];

save_undo();
/* 1st get initial angle... */
if (!rub_line())
	return(0);
t0 = -arctan(x_1-x_0, y_1-y_0);
itheta = t0+TWOPI/4;
spi_ttheta = 0;
for (;;)
	{
	spi_rad = calc_distance(x_0,y_0,grid_x,grid_y);
	t1 = -arctan(grid_x-x_0, grid_y-y_0);
	dt = t1-t0;
	if (dt > TWOPI/2)
		dt = dt-TWOPI;
	if (dt < -TWOPI/2)
		dt = dt+TWOPI;
	spi_ttheta += dt;
	t0 = t1;
	sprintf(buf, spiral_100 /* " %6ld degrees" */, spi_ttheta*360/TWOPI);
	top_text(buf);
	if (!make_spiral_poly(x_0,y_0,spi_rad,itheta,spi_ttheta))
		return(0);
	dot_poly(&working_poly, sdot);
	wait_input();
	dot_poly(&working_poly,copydot);
	if (PJSTDN || RJSTDN)
		break;
	}
restore_top_bar();
return(PJSTDN);
}

