
/* zoom.c - help simulate fat-bits.  Position zoom window on screen. */

#include "jimk.h"
#include "flicmenu.h"
#include "commonst.h"
#include "zoom.str"


Vscreen *zoom_form;

static char *zoom_choices[] =
	{
	zoom_100 /* "  Times 2 " */,
	zoom_101 /* "  Times 4" */,
	cst_cancel,
	};

static
calc_zoom_w()
{
if (vs.zoom4)
	{
	vs.zoomscale = 4;
	vs.zoomw = XMAX/4;
	vs.zoomh = YMAX/4;
	}
else
	{
	vs.zoomscale = 2;
	vs.zoomw = XMAX/2;
	vs.zoomh = YMAX/2;
	}
}

set_zoom_level()
{
char *zs;

hide_mp();
unzoom();
zoom_choices[0][0] = ' ';
zoom_choices[1][0] = ' ';
if (vs.zoom4)
	zs = zoom_choices[1];
else
	zs = zoom_choices[0];
zs[0] = '*';
switch (qchoice(zoom_103 /* "Set zoom level" */, zoom_choices, Array_els(zoom_choices) ))
	{
	case 1:
		vs.zoom4 = 0;
		break;
	case 2:
		vs.zoom4 = 1;
		break;
	}
calc_zoom_w();
clip_zoom();
rezoom();
draw_mp();
}


static
clip_zoom()
{
if (vs.zoomx < 0)
	vs.zoomx = 0;
if (vs.zoomy < 0)
	vs.zoomy = 0;
if (vs.zoomx+vs.zoomw > XMAX)
	vs.zoomx = XMAX-vs.zoomw;
if (vs.zoomy+vs.zoomh > YMAX)
	vs.zoomy = YMAX-vs.zoomh;
}

static
set_alloced_zoom()
{
copy_form(&vf,zoom_form);	/* copy unzoomed screen to zoom_form */
render_form = zoom_form;
vs.zoom_mode = 1;
make_dw();
zoom_it();
}

static
set_zoom()
{
if ((zoom_form = alloc_screen())!=NULL)
	{
	copy_form(&uf,zoom_form);	/* save undo screen */
	copy_form(&vf,&uf);		/* save undo for marqi routines */
	rub_in_place(vs.zoomx,vs.zoomy,vs.zoomx+vs.zoomw-1,vs.zoomy+vs.zoomh-1);
	if (PJSTDN)	/* move zoom box... */
		{
		move_box(&vs.zoomx,&vs.zoomy,vs.zoomw,vs.zoomh);
		clip_zoom();
		}
	copy_form(zoom_form, &uf);	/* replace undo buffer */
	set_alloced_zoom();
	return(1);
	}
else
	{
	return(0);
	}
}


unset_zoom()
{
copy_form(zoom_form, &vf);	
free_screen(zoom_form);
zoom_form = NULL;
vs.zoom_mode = 0;
render_form = &vf;
make_dw();
}


toggle_zoom(m)
Flicmenu *m;
{
hide_mp();
if (zoom_form)
	unset_zoom();
else
	set_zoom();
draw_mp();
draw_sel(m);
make_wi_list();
}

ktoggle_zoom()
{
if (zoom_form)
	unset_zoom();
else
	set_zoom();
make_wi_list();
}



static WORD lzoom_mode, zstack;

/* temporarily get out of zoom (used by system not user) */
unzoom()
{
if (zstack == 0)
	{
	if (vs.zoom_mode)
		{
		lzoom_mode = vs.zoom_mode;
		unset_zoom();
		}
	}
zstack++;
}

/* go back to zoom from temporary suspension */
rezoom()
{
if (--zstack == 0)
	{
	if (lzoom_mode)
		{
		if ((zoom_form = alloc_screen() )!=NULL)
			{
			set_alloced_zoom();
			}
		lzoom_mode = 0;
		}
	}
}

init_zoom()
{
calc_zoom_w();
if (vs.zoom_mode)
	if ((zoom_form = alloc_screen() )!=NULL)
		set_alloced_zoom();
}
