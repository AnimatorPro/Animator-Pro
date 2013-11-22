
/* pentools.c - routines that recieve input when the cursor is over the
   drawing screen (and not in a sub-menu).  */

#include "jimk.h"

extern WORD x_0,y_0,x_1,y_1;

/* some functions for line drawing */
extern int copydot(),marqidot(),sdot(), xdot();
extern a1bdot(), rbdot(), rbbrush();
/* some functions for circle drawing */
extern int sdot(), copydot();

box_tool()
{
int ocolor;

if (!vs.fillp)
	brushcursor = dot_pens[vs.pen_width];
if (!pti_input())
	return;
save_undo();
rub_box();
swap_box();
if (PJSTDN)
	{
	if (vs.cycle_draw) cycle_ccolor();
	if (vs.fillp)
		{
		render_box(x_0,y_0,x_1,y_1);
		if (vs.color2)
			{
			ocolor = vs.ccolor;
			vs.ccolor = vs.inks[7];
			render_frame(x_0,y_0,x_1,y_1);
			vs.ccolor = ocolor;
			}
		}
	else
		render_frame(x_0,y_0,x_1,y_1);
	}
}


fill_tool()
{
if (!pti_input())
	return;
save_undo();
if (vs.cycle_draw) cycle_ccolor();
fill(grid_x, grid_y);
}

flood_tool()
{
int fcolor;

if (!pti_input())
	return;
fcolor = dwgetdot(grid_x, grid_y);
wait_click();
if (PJSTDN)
	{
	save_undo();
	if (vs.cycle_draw) cycle_ccolor();
	flood(grid_x, grid_y, fcolor);
	}
}

edge_tool()
{
if (!pti_input())
	return;
save_undo();
if (vs.cycle_draw) cycle_ccolor();
edge1(dwgetdot(grid_x,grid_y));
}



drizl_tool()
{
dtool(2);
}

streak_tool()
{
dtool(0);
}

draw_tool()
{
dtool(1);
}

extern WORD dot_c[], circ1_cursor[], circ2_cursor[], circ3_cursor[],
	box1_cursor[];

/* wierd tool that makes line thinner the faster you go */
static
dtool(mode)
int mode;
{
WORD *speedc;
WORD first, delt,i,j;
#define DL 3
#define DLMAX 16
#define DLHI (DLMAX*DL)
WORD delts[DL];
WORD ix;
WORD around;
WORD open_width;
WORD lx,ly;
WORD drx,dry;

brushcursor = dot_pens[vs.pen_width];
for (;;)
	{
	if (!pti_input())
		return;
	open_width = vs.pen_width;	/* drizzle may change this */
	render_full_screen();
	if (!make_render_cashes())
		return;

	/* deal with initializing speed sampling buffer for drizzle only */
	for (i=0; i<DL; i++)
		delts[i] = DLMAX;

	/* save screen to undo before user blows it */
	save_undo();

	/* Will be dealing with two points at a time for lines.  Make the first
	   line just one dot to fake it. */
	lx = grid_x;
	ly = grid_y;

	/* set up drizzle last x/y */
	drx = uzx;
	dry = uzy;

	/* some misc counters */
	i=0;
	around = 0;

	for (;;)
		{
		if (vs.cycle_draw) cycle_ccolor();
		switch (mode)
			{
			case 0:	
				render_brush(grid_x,grid_y);
				break;
			case 1:
				render_line(lx,ly,grid_x,grid_y);
				break;
			case 2:
				if (pressure_sensitive)
					{
					vs.pen_width = ((open_width*pressure)>>8);
					}
				else
					{
					if (around <= 0)	/* only do it every 4th time around */
						{
						delt = 0;
						for (j=0; j<DL; j++)
							delt += delts[j];
						if (delt >= DLHI)
							vs.pen_width = 0;
						else
							vs.pen_width = 
								(DLHI/2 + (DLHI-delt)*open_width)/(DLHI);
						delts[i] = intabs(drx-uzx) + intabs(dry-uzy);
						drx = uzx;
						dry = uzy;
						i++;
						if (i >= DL)
							i = 0;
						around = 4;
						}
					}
				render_line(lx,ly,grid_x,grid_y);
				--around;
				break;
			}
		lx = grid_x;
		ly = grid_y;
		if (mode == 1)
			wait_input();
		else
			vsync_input(1);
		if (!PDN)
			break;
		}
	free_render_cashes();
	vs.pen_width = open_width;
	}
}


spray_tool()
{
short xy[2];
char rgb[3];
int i,roff=0;
int spread;

brushcursor = dot_pens[vs.pen_width];
for (;;)
	{
	if (!pti_input())
		return;
	render_full_screen();
	if (!make_render_cashes())
		return;
	save_undo();
	while (PDN)
		{
		if (pressure_sensitive)
			{
			i = ((vs.air_speed*pressure)>>8);
			if (i < 1)
				i = 1;
			spread = (vs.air_spread>>1) + ((vs.air_spread*pressure)>>9);
			if (spread < 1)
				spread = 1;
			}
		else
			{
			i = vs.air_speed;
			spread = vs.air_spread;
			}
		while (--i >= 0)
			{
			polar( random()+roff++, random()%spread, xy);
			xy[0]+=grid_x;
			xy[1]+= grid_y;
			if (vs.cycle_draw) cycle_ccolor();
			render_brush(xy[0], xy[1]);
			}
		vsync_input(2);
		}
	free_render_cashes();
	}
}

circle_tool()
{
int ocolor;

if (!vs.fillp)
	brushcursor = dot_pens[vs.pen_width];
if (!pti_input())
	return;
save_undo();
if (rub_circle())
	{
	if (vs.cycle_draw) cycle_ccolor();
	if (vs.fillp)
		{
		render_disk(x_0,y_0,center_rad());
		if (vs.color2)
			{
			ocolor = vs.ccolor;
			vs.ccolor = vs.inks[7];
			render_circle(x_0, y_0, center_rad());
			vs.ccolor = ocolor;
			}
		}
	else
		render_circle(x_0, y_0, center_rad());
	}
}



line_tool()
{
WORD firstx,firsty;
WORD lx,ly;

brushcursor = dot_pens[vs.pen_width];
if (!pti_input())
	return;
save_undo();
if (rub_line())
	{
	if (vs.cycle_draw) cycle_ccolor();
	render_full_screen();
	if (!make_render_cashes())
		return;
	render_1_line(x_0,y_0,x_1, y_1);
	free_render_cashes();
	}
}
