/* grid.c - Stuff to implement functions in extras/grid...
 * 'Snap' input to fall on intersections of a user definable
 * grid.  Routines to see and paste the grid too. */

#include "jimk.h"
#include "auto.h"
#include "errcodes.h"
#include "flicel.h"
#include "menus.h"
#include "render.h"
#include "softmenu.h"

static void go_gridreq(Boolean keep_undo);

USHORT constrain_angle(SHORT angle)
/* note this accepts angles in FCEL_TWOPI units */
{
	if(vs.rot_grid > 1)
		return(((angle + vs.rot_grid/2)/vs.rot_grid)*vs.rot_grid);
	else
		return(angle);
}
static Errcode
paste1_grid(void *data, int ix, int intween, int scale, Autoarg *aa)
/* auto-function to paste  grid  onto picture */
{
	Errcode err;
	SHORT x, y;
	(void)data;
	(void)ix;
	(void)intween;
	(void)scale;
	(void)aa;

	set_full_gradrect();
	if((err = make_render_cashes()) < 0)
		goto error;
	start_abort_atom();
	for (x=vl.grid.x; x<vb.pencel->width; x+=vl.grid.width)
	{
		pj_cline(x, 0, x, vb.pencel->height, render_dot, NULL);
		if((err = poll_abort()) < Success)
			goto aborted;
	}
	for (y=vl.grid.y; y<vb.pencel->height; y+=vl.grid.height)
	{
		if((err = poll_render_hline(y, 0, vb.pencel->width,
									(Raster *)vb.pencel)) < Success)
		{
			goto aborted;
		}
	}
aborted:
	err = errend_abort_atom(err);
	free_render_cashes();
error:
	return(err);
}
void qgrid_keep_undo(void)
{
	go_gridreq(TRUE);
}
void qgrid(void)
{
	/* this is a kludge. all the things that
	 * use overlays also need the undo buffer and pasting or altering the
	 * undo will make it out of sync */
	go_gridreq(flxtime_data.draw_overlays != NULL);
}

static void draw_grid(void)
{
SHORT x, y;

	for (x=vl.grid.x; x < vb.pencel->width; x+=vl.grid.width)
	{
		pj_set_vline(vb.pencel, vs.ccolor, x, 0, vb.pencel->height);
	}
	for (y=vl.grid.y; y < vb.pencel->height; y+=vl.grid.height)
	{
		pj_set_hline(vb.pencel, vs.ccolor, 0, y, vb.pencel->width);
	}
}

static void see_grid(void)
{
	save_undo();
	draw_grid();
	zoom_it();
	wait_wndo_input(ANY_CLICK);
	zoom_unundo();
}
static void make_grid(void)
{
	save_undo();
	vs.use_grid = 0;

	if((gcut_out_rect(&vl.grid)) >= 0)
	{
		if(!vl.grid.width)
			vl.grid.width = 1;
		if (!vl.grid.height)
			vl.grid.height = 1;
		vl.grid.x = vl.grid.x % vl.grid.width;
		vl.grid.y = vl.grid.y % vl.grid.height;

		/* move changed values into vs buffer */

		vs.gridx = scale_vscoor(vl.grid.x,vb.pencel->width);
		vs.gridw = scale_vscoor(vl.grid.width,vb.pencel->width);
		vs.gridy = scale_vscoor(vl.grid.y,vb.pencel->height);
		vs.gridh = scale_vscoor(vl.grid.height,vb.pencel->height);

		vs.use_grid = 1;
	}
	load_wndo_iostate(NULL); /* screen settings */
	see_grid();
}
static void do_qfunc(VFUNC gfunc, Boolean keep_undo)
{
Rcel_save undosave;

	if(keep_undo)
	{
		if(report_temp_save_rcel(&undosave,undof) < Success)
			return;
	}
	(*gfunc)();
	if(keep_undo)
	{
		report_temp_restore_rcel(&undosave,undof);
	}
}
static void go_gridreq(Boolean keep_undo)
/* put  up  numbered choice menu for grid certain items are disabled or altered
 * for overlayed environs that need the undo buffer in sync with the screen */
{
int choice;
USHORT gdis[6];
SHORT angle;

	hide_mp();
	for (;;)
		{
		clear_mem(gdis, sizeof(gdis));
		if (vs.use_grid)
			gdis[0] = QCF_ASTERISK;

		if(keep_undo)
			gdis[2] |= QCF_DISABLED; /* no pasting !! */

		if ((choice = soft_qchoice(gdis, "grid")) < Success)
			break;
		switch (choice)
			{
			case 0:	/* use grid */
				vs.use_grid = !vs.use_grid;
				break;
			case 1: /* make grid */
				do_qfunc(make_grid,keep_undo);
				break;
			case 2: /* paste grid */
				uzauto(paste1_grid, NULL);
				break;
			case 3: /* see grid */
				do_qfunc(see_grid,keep_undo);
				break;
			case 4: /* set rot grid */
				angle = 0.5 + ((360.0*(FLOAT)vs.rot_grid)/FCEL_TWOPI);
				if(soft_qreq_number(&angle,0,359,"rot_grid"))
					vs.rot_grid = 0.5 + (((FLOAT)angle*FCEL_TWOPI)/360.0);
				break;
			default:
				goto OUT;
			}
		}
OUT:
	show_mp();
}
void grid_flixy(SHORT *flix, SHORT *fliy)
{
	*flix = (((*flix - vl.grid.x + vl.grid.width/2)/vl.grid.width)
				*vl.grid.width) + vl.grid.x;
	*fliy = (((*fliy - vl.grid.y + vl.grid.height/2)/vl.grid.height)
				*vl.grid.height) + vl.grid.y;
}
