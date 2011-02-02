
/* vpsubs.c - routines used by all sort of menu panels.  Really a mismatch.
   Not well suited to being in an overlay. */

#include "jimk.h"
#include "flicmenu.h"
#include "fli.h"

extern Flicmenu dsel1_sel, it0_sel; /* first pen tool slot */
extern Pull macro_pull, cel_pull, window_pull;
extern Option_list *options_list;

cluster_count()
{
return(vs.buns[vs.use_bun].bun_count);
}

UBYTE *
cluster_bundle()
{
return(vs.buns[vs.use_bun].bundle);
}

/* convert from alphabetized menu order to historical implementation
   order */
UBYTE dmd_lookup[] =
	{
	10,	/* Add */
	17,	/* Bright */
	20, /* Close */
	21, /* Dark */
	22, /* Emboss */
	6,  /* Glass */
	4,	/* Glaze */
	11, /* Glow */
	18, /* Grey */
	2,	/* H Grad */
	16, /* Hollow */
	9,	/* Jumble */
	3,	/* L Grad */
	0,	/* Opaque */
	23, /* Pull */
	25, /* R Grad */
	7,	/* Scrape */
	24, /* Smear */
	5,  /* Soften */
	13, /* Spark */
	14, /* Split */
	19, /* Sweep */
	12, /* Tile */
	15,	/* Unzag */
	1,  /* V Grad */
	8,	/* Xor */
	};
/* and convert back */
UBYTE idmd_lookup[sizeof(dmd_lookup)];


static
make_idmd_lookup()
{
int i;

for (i=0; i<sizeof(dmd_lookup); i++)
	idmd_lookup[dmd_lookup[i]] = i;
}

static
attatch_dm()
{
extern Flicmenu dms0_sel;
extern Option_list *dm_list;
int i;
int t;
Flicmenu *f;
Option_list *o;
UBYTE *c;

make_idmd_lookup();
f = &it0_sel;
c = vs.dm_slots;
i = 6;
while (--i >= 0)
	{
	t = *c++;
	o = list_el(dm_list,idmd_lookup[t]);
	f->identity = t;
	f->text = o->name;
	f = f->next;
	}
}


static
attatch_tools()
{
int i;
int t;
Flicmenu *f;
Option_list *o;

f = &dsel1_sel;
for (i=0; i<6; i++)
	{
	t = vs.tool_slots[i];
	o = list_el(options_list,t);
	f->identity = t;
	f->text = o->name;
	f = f->next;
	}
}


rethink_settings()
{
attatch_tools();
attatch_dm();
clip_rmove_menu(cur_menu, 0, vs.rmyoff-cur_menu->y);
change_dir(vs.drawer);
check_cel_options();
}

default_settings()
{
copy_structure(&default_vs, &vs, sizeof(vs) );
copy_cmap(init_cmap, render_form->cmap);
rethink_settings();
if (zoom_form)
	unset_zoom();
}


in_ctable(color, table, count)
UBYTE color;
UBYTE *table;
int count;
{
while (--count >= 0)
	{
	if (color == *table++)
		return(1);
	}
return(0);
}

restore()
{
save_undo();
fli_abs_tseek(render_form,vs.frame_ix);
see_cmap();
zoom_it();
dirty_frame = 0;
}

#ifdef SLUFFED
mrestore()
{
hide_mp();
restore();
draw_mp();
}
#endif SLUFFED

mundo_pic()
{
hide_mp();
undo_pic();
draw_mp();
}


undo_pic()
{
swap_undo();
see_cmap();
zoom_it();
dirties();
}

init_seq()
{
vs.frame_ix = 0;
clear_form(render_form);
see_cmap();
}

kill_seq()
{
save_undo();
init_seq();
empty_tempflx();
vs.bframe_ix = 0; /* back frame buffer no good now */
}

flush_tempflx()
{
scrub_cur_frame();
f_tempflx();
}

static
cleanup()
{
/* delete back buffer screen */
jdelete(bscreen_name);
vs.bframe_ix = 0;
/* update tempflx header and stuff */
flush_tempflx();
/* Close any pending macro recording */
close_macro();
/* push a copy of current screen and alt,cel etc for when program started 
   again... */
push_pics();
uninit_sys();
}

outofhere()
{
cleanup();
exit(0);
}

toggle_menu()
{
if (cur_menu != NULL)
	{
	cur_pull = NULL;
	cur_menu = NULL;
	}
else
	{
	cur_pull = &root_pull;
	cur_menu = &quick_menu;
	}
}

defrag()
{
unzoom();
push_pics();
f_tempflx();
close_tflx();
open_tempflx();
pop_pics();
rezoom();
}

interp_range(c1, c2,  i, divi)
int c1, c2, i, divi;
{
if (divi == 1)
	return(c1);
else
	divi-=1;
return( (c2*i + c1*(divi-i) + divi/2)/divi);
}

