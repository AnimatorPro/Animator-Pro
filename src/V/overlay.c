
/* overlay.c - handle requests off the "composite..." menu.  Stuff to
   mix two FLICS onto the same screen and eventually same file.  */

#include "jimk.h"
#include "fli.h"
#include "flicmenu.h"
#include "inks.h"
#include "commonst.h"
#include "overlay.str"

extern char under_flag;

static char *overlay_options[] = {
	NULL,
	NULL,
	overlay_100 /* "Cross-fade" */,
	cst_cancel,
	};

static char *cmap_options[] ={
	overlay_102 /* "Combine color maps." */,
	overlay_103 /* "Keep current colors." */,
	overlay_104 /* "Use incoming colors." */,
	overlay_105 /* "No Fitting" */,
	cst_cancel,
	};

extern char *ink_word();

qload_overlay()
{
int how;
char buf1[30];
char buf2[30];
char *iname;

iname = ink_word();
sprintf(buf1, overlay_107 /* "Overlay %s" */, iname);
sprintf(buf2, overlay_108 /* "Underlay %s" */, iname);
overlay_options[0] = buf1;
overlay_options[1] = buf2;
how = qchoice(overlay_109 /* "Composite functions" */, 
	overlay_options, Array_els(overlay_options) );
if (how != 0)
	{
	unzoom();
	push_inks();
	ink_push_cel();
	load_overlay(how-1);
	ink_pop_cel();
	pop_inks();
	rezoom();
	}
}

/* overlay goes past end of current fli */
static char *wrap_choices[] = {
	overlay_110 /* "Wrap at end" */,
	overlay_111 /* "Chop at end" */,
	};




static char *overl_msgs[3] = {
	overlay_112 /* "Composite overlay" */,
	overlay_113 /* "Composite underlay" */,
	overlay_114 /* "Composite cross fade" */,
	};

static 
load_overlay(how)
int how;
{
char *title;
int oink, ostrength, oclear;

ostrength = vs.tint_percent;
oink = vs.draw_mode;
oclear = vs.zero_clear;
if ((title = get_filename(overl_msgs[how], ".FLI"))!=NULL)
	{
	if (how == 1)	/* underlay */
		{
		how = 0;
		under_flag = 1;
		}
	load_fli_overlay(title, how);
	under_flag = 0;
	}
vs.tint_percent = ostrength;
vs.draw_mode = oink;
vs.zero_clear = oclear;
}

static
may_cfit_blit_cel(cel, fit)
Vscreen *cel;
int fit;
{
if (fit == 0 || fit == 4 || fit == 3)
	{
	rblit_cel(cel);
	}
else
	{
	if (!cfit_rblit_cel(cel))
		return(0);
	}
return(1);
}


static
load_fli_overlay(title, how)
char *title;
int how;
{
Vscreen *lscreen;
Vscreen *unders;
Vscreen ouf;
int ret;
Vcel vcel;
Vcel *ocel;
int fd;
struct fli_head flih;
int i;
int fit_option;
int usr_fit;
int xoff, yoff;
int oink0;
PLANEPTR cmap;
int clear, clearcolor, tinting;
WORD ozc;
int fcount;

if (!save_pic(screen_name, render_form,0))
	return(0);
ret = 0;	/* more ways to fail than succeed sadly */
usr_fit = 0;	/* start off not knowing how to fit colors */
if ((fd = read_fli_head(title, &flih)) == 0)
	return(0);
lscreen = NULL;
cmap = NULL;
if ((lscreen = alloc_screen()) == NULL)
	goto OUT;
if ((cmap = begmem(COLORS*3)) == NULL)
	goto OUT;
for (i=0; i<flih.frame_count; i++)
	{
	if (i != 0 && vs.frame_ix == 0)
		{
		if (qchoice(overlay_116 /* "Composite past end?" */, 
			wrap_choices, Array_els(wrap_choices))
			!= 1)
			{
			goto OUT;
			}
		}
	if (!read_next_frame(title,fd,lscreen,0))
		goto OUT;
	if (!cmaps_same(lscreen->cmap, render_form->cmap))
		{
		if (!usr_fit)
			{
			if ((usr_fit = 
				qchoice(overlay_117 /* "What about the color maps?" */,
				cmap_options, Array_els(cmap_options))) == 0)
				goto OUT;
			}
		fit_option = usr_fit;
		}
	else
		fit_option = 0;
	/* switch to get the color map into cmap */
	switch (fit_option)
		{
		case 0:	/* both are same.  yea! */
		case 4:	/* No Fit */
		case 2:	/* keep current */
			copy_cmap(render_form->cmap,cmap);
			break;
		case 3: /*use overlay cmap */
			copy_cmap(lscreen->cmap, cmap);
			break;
		case 1: /* compromise cmap */
			compromise_cmap(render_form->cmap, lscreen->cmap, cmap);
			break;
		}
	/* switch to color fit render_form if necessary */
	switch (fit_option)
		{
		case 0:	/* both are same.  yea! */
		case 4:	/* No Fit */
		case 2:	/* keep current */
			break;
		case 3: /*use overlay cmap */
		case 1: /* compromise cmap */
			screen_to_cel( render_form, &vcel);
			cfit_cel(&vcel,cmap);
			copy_cmap(cmap,render_form->cmap);
			see_cmap();
			break;
		}
	screen_to_cel( lscreen, &vcel);
	if (i == 0)  /* let user position first frame */
		{
		ocel = cel;
		cel = &vcel;
		move_cel();
		cel = ocel;
		xoff = vcel.x;
		yoff = vcel.y;
		if (!yes_no_line(
			overlay_118 /* "Composite flic starting this frame?" */))
			{
			load_some_pic(screen_name, render_form);
			see_cmap();
			goto OUT;
			}
		/* we're committed now */
		jdelete(screen_name);
		dirtyf();
		dirty_frame = 0;
		}
	vcel.x = xoff;
	vcel.y = yoff;
	switch (how)
		{
		case 0:	 /* over/under */
			copy_form(render_form, &uf);
			if (!may_cfit_blit_cel(&vcel, fit_option))
				goto OUT;
			break;
		case  2: /* crossfade */
			vs.draw_mode = I_GLASS;
			if ((fcount = flih.frame_count-1) == 0)
				fcount = 1;
			vs.tint_percent = (i*(long)100+flih.frame_count/2) / fcount;
			vs.zero_clear = 0;
			copy_form(render_form, &uf);
			if (!may_cfit_blit_cel(&vcel, fit_option))
				goto OUT;
			break;
		}
	if (!sub_cur_frame())
		goto OUT;
	if (check_abort(i+1, flih.frame_count))
		goto OUT;
	vs.frame_ix++;
	check_loop();
	if (!unfli(render_form,vs.frame_ix,1))
		goto OUT;
	}
ret = 1;
OUT:
gentle_freemem(cmap);
free_screen(lscreen);
jclose(fd);
return(ret);
}



