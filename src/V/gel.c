
/* gel.c - Stuff to do the gel tool.  Basiccally concentric transparent
  circles with the wider ones thinner. */

#include "jimk.h"

extern WORD x_0,y_0,x_1,y_1;

/* some functions for line drawing */
extern int copydot(),marqidot(),sdot(), xdot();
extern a1bdot(), rbdot(), rbbrush();
/* some functions for circle drawing */
extern int xysdot(), xycopydot();
extern render_hline();



extern WORD gel_factor;
extern WORD gel_input;
extern char *gel_thash;

static int gft[4] = {15*1, 16*2, 17*3, 18*4};
static char *th[4];

static
zero_th()
{
int i;

for (i=0; i<4; i++)
	zero_structure(th[i], COLORS*2);
}

gel_tool()
{
int pw;
int pwt[4];
int i;

brushcursor = dot_pens[vs.pen_width];
pw = vs.pen_width+1;
for (;;)
	{
	if (!pti_input())
		return;
	zero_structure(th, sizeof(th));
	render_full_screen();
	if (vs.draw_mode == 0)	/* opaque ... can optimize */
		{
		for (i=0; i<4;i++)
			{
			if ((th[i] = begmem(COLORS*2)) == NULL)
				goto CLEANUP;
			}
		zero_th();
		}
	else
		{
		make_render_cashes();
		if (!is_bhash())
			make_bhash();
		}
	gel_input = 1;
	save_undo();
	render_full_screen();
	while (PDN)
		{
		if (pressure_sensitive)
			{
			pw = ((vs.pen_width*pressure)>>8)+1;
			}
		pwt[0] = pw*2;
		pwt[1] = pw*3/2;
		pwt[2] = pw;
		pwt[3] = pw/2;
		for (i=0; i<4; i++)
			{
			gel_factor = gft[i];
			gel_thash = th[i];
			ccircle(grid_x,grid_y,pwt[i],NULL,render_hline,TRUE);
			}
		if (vs.cycle_draw)
			{
			cycle_ccolor();
			if (vs.draw_mode == 0)
				zero_th();
			else
				{
				if (!is_bhash())
					make_bhash();
				}
			}
		check_input();
		}
	CLEANUP:
	if (vs.draw_mode == 0)
		{
		for (i=0; i<4; i++)
			gentle_freemem(th[i]);
		}
	else
		{
		free_render_cashes();
		if (is_bhash())
			free_bhash();
		}
	gel_factor = 0;
	gel_input = 0;
	}
}

