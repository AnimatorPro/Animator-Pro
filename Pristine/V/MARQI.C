
/* marqi.c - routines to make creepy dotted lines around selected areas. */
/* Also some "Dotout" family functions that take x/y parameters and act
   on render_form.  Dotout's are used by line drawers, circle drawers etc. */

#include "jimk.h"
#include "flicmenu.h"
#include "marqi.str"

/* some functions for line drawing */
extern int copydot(),marqidot(),sdot(), xdot();
extern a1bdot();
extern WORD *abehind, *mbehind;

/* Some global variables used by various coordinate parsing routines
   (rub-box(), rub-line(), etc. */
int cenx, ceny, rad;
WORD x_0,y_0,x_1,y_1;

/* Structure to keep the dots of a creepy marqi coherent and always
   creeping in the same direction.  Mostly works too. */
struct marqidata marqidata;
static int lmod;

/* "dotout" routine to erase from undo screen */
copydot(x,y)
int x,y;
{
register int off;


off = y*uf.bpr+x;
if (vs.zoom_mode)
	upd_zoom_dot(x,y,uf.p[off]);
else if (!on_menus(y) )
	{
	if (x >= 0 && x < XMAX && y >= 0 && y < YMAX)
		{
		render_form->p[off] = uf.p[off];
		}
	}
}


/* dotout to set pixel with current color. */
sdot(x,y)
int x,y;
{
if (vs.zoom_mode)
	upd_zoom_dot(x,y,vs.ccolor);
else if (!on_menus(y) )
	{
	if (x >= 0 && x < XMAX && y >= 0 && y < YMAX)
		{
		render_form->p[y*render_form->bpr+x] = vs.ccolor;
		}
	}
}

/* dotout for creepy marqi users */
marqidot(x,y)
int x,y;
{
int color;

color = ((--marqidata.mod&7) < 4 ? swhite : sblack);
if (vs.zoom_mode)
	upd_zoom_dot(x,y,color);
else if (!on_menus(y) )
	{
	if (x >= 0 && x < XMAX && y >= 0 && y < YMAX)
		{
		render_form->p[y*render_form->bpr+x] = color;
		}
	}
}



/* transform coordinates firstx/grid_x to a pair x_0/x_1 where it's
   gauranteed x_0 <= x_1 and likewise for y. */
swap_box()
{
if (firstx > grid_x)
	{
	x_0 = grid_x;
	x_1 = firstx;
	}
else
	{
	x_0 = firstx;
	x_1 = grid_x;
	}
if (firsty > grid_y)
	{
	y_0 = grid_y;
	y_1 = firsty;
	}
else
	{
	y_0 = firsty;
	y_1 = grid_y;
	}
}

/* rub_line - display a marqi line until next click.  Display many
   meaningful number on top lines. */
rub_line()
{
char buf[60];

x_0 = grid_x;
y_0 = grid_y;
for (;;)
	{
	x_1 = grid_x;
	y_1 = grid_y;
	cline( x_0,y_0,x_1,y_1,sdot);	/* draw new */
	sprintf(buf, marqi_100 /* " (%3d %3d) wid %3d hgt %3d (%3d %3d) deg %3ld rad %3d" */,
		x_0, y_0, intabs(x_0-x_1)+1, intabs(y_0-y_1)+1, x_1, y_1,
		(360L*arcnorm(arctan(x_1-x_0, y_1-y_0))+TWOPI/2)/TWOPI, 
		calc_distance(x_0, y_0, x_1, y_1));
	top_text(buf);
	wait_input();
	cline( x_0,y_0,x_1,y_1,copydot);	/* undo last */
	if (PJSTDN||RJSTDN)
		break;
	}
restore_top_bar();
return(PJSTDN);
}

/* Draw a hollow rectangle through dotout routine */
some_frame(x0,y0,x1,y1,dotout)
int x0,y0,x1,y1;
Vector dotout;
{
cline( x0, y0, x1, y0, dotout);
cline( x1, y0, x1, y1, dotout);
cline( x1, y1, x0, y1, dotout);
cline( x0, y1, x0, y0, dotout);
}

/* Draw full-screen cross-hair through dotout routine */
static
some_cut(x, y, func, marq)
int x, y, marq;
Vector func;
{
marqidata.mod = marq;
cline( 0, y, x, y, func);
marqidata.mod = marq;
cline( XMAX-1, y, x, y, func);
marqidata.mod = marq;
cline( x, 0, x, y, func);
marqidata.mod = marq;
cline( x, YMAX-1, x, y, func);
}


/* displays cut cursor until pendown... */
cut_cursor()
{
int lx,ly;
char buf[16];
int ret;

mouse_on = 0;
clickonly = 1;
for (;;)
	{
	lx = grid_x;
	ly = grid_y;
	vsync_input(4);
	if (mouse_moved || key_hit)
		{
		some_cut(lx,ly,copydot, lmod);
		sprintf(buf, " %3d %3d", grid_x, grid_y);
		top_text(buf);
		}
	if (PJSTDN)
		{
		ret = 1;
		break;
		}
	if (key_hit || RJSTDN)
		{
		ret = 0;
		break;
		}
	some_cut(grid_x, grid_y, marqidot, lmod++);
	}
restore_top_bar();
mouse_on = 1;
clickonly = 0;
return(ret);
}

/* marqui rectangle until pendown */
r_in_place(x0,y0,x1,y1)
int x0,y0,x1,y1;
{
clickonly = 1;
for (;;)
	{
	marqidata.mod = lmod++;
	marqi_frame(x0, y0, x1, y1);
	vsync_input(4);
	if (PJSTDN || RJSTDN || key_hit)
		break;
	}
undo_frame(x0, y0, x1, y1);
restore_top_bar();
clickonly = 0;
return(PJSTDN);
}

/* marqui rectangle until pendown displaying rectangle coordinates */
rub_in_place(x0,y0,x1,y1)
int x0,y0,x1,y1;
{
box_coors(x0,y0,x0,y0);
r_in_place(x0,y0,x1,y1);
}

/* rub box.  Called at one pendown.  Rub's a box intil next pendown... 
  erasing from the undo buffer... */
static
rbox(dotout, extra)
Vector dotout;
int extra;
{
int i;
int lx, ly;
char buf[40];

firstx = grid_x;
firsty = grid_y;
marqidata.mod = lmod++;
clickonly = 1;
for (i=0;;i++)
	{
	lx = grid_x;
	ly = grid_y;
	vsync_input(4);
	if (mouse_moved)
		{
		undo_frame(lx, ly, firstx, firsty);
		sprintf(buf, " %3d %3d  (%3d %3d)  %3d %3d", firstx, firsty,
			intabs(firstx-grid_x)+extra, intabs(firsty-grid_y)+extra,
			grid_x, grid_y);
		top_text(buf);
		}
	if (PJSTDN || RJSTDN)
		{
		break;
		}
	marqidata.mod = lmod++;
	some_frame(grid_x, grid_y, firstx, firsty,dotout);
	}
clickonly = 0;
restore_top_bar();
return(PJSTDN);
}

/* rub box using marqi'ing */
rub_box()
{
return(rbox(marqidot,1));
}

/* rub grid box */
static
rub_gbox()
{
return(rbox(marqidot,0));
}

/* Define a box on the screen.  Start with a cross-hair cursor. */
gcut_out()
{
if (cut_cursor())
	{
	if (rub_gbox())
		{
		swap_box();
		return(1);
		}
	}
return(0);
}


/* rub box using solid current color */
rs_box()
{
return(rbox(sdot,1) );
}

/* display a point-list through dotout */
msome_vector(pts, count, dotout, data, open)
Point *pts;
int count;
Vector dotout;
void *data;
int open;
{
Point *last;

if (open)
	{
	last = pts;
	pts++;
	count -= 1;
	}
else
	{
	last = pts+count-1;
	}
marqidata.mod = lmod++;
while (--count >= 0)
	{
	cline( last->x, last->y, pts->x, pts->y, dotout);
	last = pts;
	pts++;
	}
}

static
msome_poly(pts, count, dotout, data)
Point *pts;
int count;
Vector dotout;
void *data;
{
msome_vector(pts, count, dotout, data, 0);
}


/* marqi a point list */
marqi_poly(pts, count)
Point *pts;
int count;
{
msome_poly(pts, count, marqidot, &marqidata);
}

/* erase a point list */
undo_poly(pts, count)
Point *pts;
int count;
{
msome_poly(pts, count, copydot, uf.p);
}

/* display coordinates and how much a box moved*/
box_coors(x,y,ox,oy)
int x,y,ox,oy;
{
char buf[40];

sprintf(buf, " (%3d %3d ) (%4d %4d )  ", x,y,x-ox,y-oy);
top_text(buf);
}

/* Move box.  Will display marqi-ing box following cursor until next click */
move_box(x0,y0,w,h)
WORD *x0, *y0, w, h;
{
WORD x, y,lx,ly;

x = *x0;
y = *y0;
lx = grid_x;
ly = grid_y;
for (;;)
	{
	box_coors(x,y,*x0,*y0);
	marqi_frame(x, y, x+w-1, y+h-1);
	vsync_input(4);
	if (mouse_moved)
		{
		undo_frame(x, y, x+w-1, y+h-1);
		x += grid_x - lx;
		y += grid_y - ly;
		if (PJSTDN || RJSTDN)
			break;
		}
	lx = grid_x;
	ly = grid_y;
	}
restore_top_bar();
if (PJSTDN)
	{
	*x0 = x;
	*y0 = y;
	return(1);
	}
else
	return(0);
}

/* Define a box on the screen.  Start with a cross-hair cursor. */
cut_out()
{
if (cut_cursor())
	{
	if (rub_box())
		{
		swap_box();
		return(1);
		}
	}
return(0);
}

/* given a fixed point on circles perimeter, make circle follow mouse
   with other end until click */
rub_circle_diagonal()
{
int bk = 0;

x_0 = grid_x;
y_0 = grid_y;
for (;;)
	{
	x_1 = grid_x;
	y_1 = grid_y;
	cenx = (x_0+x_1)/2;
	ceny = (y_0+y_1)/2;
	rad = calc_distance(x_1,y_1,x_0,y_0)/2;
	if (bk)
		break;
	ccircle(cenx,ceny,rad,sdot,NULL,FALSE);
	wait_input();
	ccircle(cenx,ceny,rad,copydot,NULL,FALSE);
	if (PJSTDN || RJSTDN)
		bk=1;
	}
return(PJSTDN);
}

/* pen tool initial input  -  basically checks if you've clicked over
   menus or over drawing space, and bails back to menus when necessary. */
pti_input()
{
clickonly = 1;
for (;;)
	{
	if (PJSTDN)
		{
		dirties();
		clickonly = 0;
		return(1);
		}
	else if (key_hit || in_control_space() || RJSTDN)
		{
		reuse_input();
		clickonly = 0;
		return(0);
		}
	wait_input();
	}
}

/* figure out radius of circle from center and current mouse position */
center_rad()
{
return(calc_distance(x_0,y_0,grid_x,grid_y));
}

/* Display a circle from click at x_0,y_0 to cursor until next click */
rub_circle()
{
int rad;

x_0 = grid_x;
y_0 = grid_y;

for (;;)
	{
	rad = center_rad();
	ccircle(x_0,y_0,rad,sdot,NULL,FALSE);
	wait_input();
	ccircle(x_0,y_0,rad,copydot,NULL,FALSE);
	if (PJSTDN || RJSTDN)
		break;
	}
return(PJSTDN);
}

/* figure out which quadrant of box the cursor is in.  Returns 0-8 */
quad9(x,y,w,h)
int x,y,w,h;
{
int quad;

if (w < 0)
	{
	x += w;
	w = -w;
	}
if (h < 0)
	{
	y += h;
	h = -h;
	}
if (grid_y < y)
	quad = 0;
else if (grid_y <= y + h)
	quad = 3;
else
	quad = 6;
if (grid_x < x)
	;
else if (grid_x <= x + w)
	quad += 1;
else
	quad += 2;
return(quad);
}

