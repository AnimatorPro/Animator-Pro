
/* grid.c - Stuff to implement functions in extras/grid...
   'Snap' input to fall on intersections of a user definable
   grid.  Routines to see and paste the grid too. */

#include  "jimk.h"
#include "flicmenu.h"
#include "grid.str"

static char *grid_options[] = {
		grid_100 /* " use" */,
		grid_101 /* " create" */,
		grid_102 /* " paste" */,
		grid_103 /* " view" */,
		grid_104 /* " Exit Menu" */,
		};

extern int quse_grid(), qmake_grid(), qpaste_grid(), qsee_grid(),
	close_menu();

static Vector grid_feelers[] =
	{
	quse_grid,
	qmake_grid,
	qpaste_grid,
	qsee_grid,
	close_menu,
	};

static 
grid_asterisks()
{
char *pt;

pt = grid_options[0];
if (vs.use_grid)
	pt[0] = '*';
else
	pt[0] = ' ';
}

qgrid()
{
grid_asterisks();
qmenu(grid_105 /* "Grid Snap Control" */, 
	grid_options, Array_els(grid_options), grid_feelers);
}

static
paste1_grid()
{
int x, y;

render_full_screen();
if (!make_render_cashes())
	return(0);
for (x=vs.gridx; x<XMAX; x+=vs.gridw)
	{
	r_box(x,0,x,YMAX-1);
	}
for (y=vs.gridy; y<YMAX; y+=vs.gridh)
	{
	r_box(0,y,XMAX-1,y);
	}
free_render_cashes();
return(1);
}

static
paste_grid()
{
int x, y;

for (x=vs.gridx; x<XMAX; x+=vs.gridw)
	{
	cvli(render_form->p, x, 0, YMAX, vs.ccolor);
	}
for (y=vs.gridy; y<YMAX; y+=vs.gridh)
	{
	chli(render_form->p, 0, y, XMAX, vs.ccolor);
	}
}

static
qpaste_grid()
{
uzauto(paste1_grid);
}

static
qsee_grid()
{
save_undo();
paste_grid();
zoom_it();
wait_click();
unundo();
zoom_it();
}

static
qmake_grid()
{
save_undo();
vs.use_grid = 0;
if (gcut_out())
	{
	vs.gridx = intmin(grid_x,firstx);
	vs.gridy = intmin(grid_y,firsty);
	if ((vs.gridw = intabs(grid_x - firstx)) <= 0)
		vs.gridw = 1;
	if ((vs.gridh = intabs(grid_y - firsty)) <= 0)
		vs.gridh = 1;
	while (vs.gridx >= 0)
		vs.gridx -= vs.gridw;
	vs.gridx += vs.gridw;
	while (vs.gridy >= 0)
		vs.gridy -= vs.gridh;
	vs.gridy += vs.gridh;
	vs.use_grid = 1;
	}
qsee_grid();
grid_asterisks();
}

static
quse_grid()
{
vs.use_grid = !vs.use_grid;
grid_asterisks();
}

