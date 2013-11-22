
/* Ellipse.c - draw an ellipse possibly with tilted axis's.  The jaggies
   on this one are still kind of ugly.  Keep promising myself to improve
   it someday.
   */

#include "jimk.h"
#include "poly.h"

extern int cenx, ceny, rad;
static int theta_offset;
static int xrad,yrad;

extern WORD x_0,y_0,x_1,y_1;
extern Poly working_poly;

/* some functions for circle drawing */
extern int sdot(), copydot();

/* some functions for line drawing */
extern int copydot(),marqidot(),sdot(), xdot();
extern a1bdot(), rbdot(), rbbrush();

static 
ell_points(bothrad)
int bothrad;
{
register int p2 = 8;

for (;;)
	{
	if (p2 > bothrad)
		return(p2);
	p2 += p2;
	}
}

ovalf_tool()
{
int bk ;

if (!vs.fillp)
	brushcursor = dot_pens[vs.pen_width];
if (!pti_input())
	return;
save_undo();
clickonly = 1;

if (!rub_circle_diagonal() )
	goto OUT;
xrad = yrad = rad;
theta_offset = 0;
bk = 0;
for (;;)
	{
	theta_offset = -arctan(grid_x - cenx, grid_y - ceny);
	xrad = calc_distance(cenx,ceny,grid_x,grid_y);
	make_sp_wpoly(cenx,ceny,xrad,theta_offset, ell_points((xrad+yrad)/2),
		WP_ELLIPSE, yrad);
	if (bk)
		break;
	some_poly(&working_poly, sdot);
	wait_input();
	some_poly(&working_poly,copydot);
	if (PJSTDN||RJSTDN)
		bk = 1;
	}
OUT:
clickonly = 0;
maybe_finish_polyt(vs.fillp);
}

