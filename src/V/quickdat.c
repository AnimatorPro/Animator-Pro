/* quickdat.c - the data structures for the main control panel.  6 tools,
   6 inks, etc.   Also some code at end for some of the buttons feelme
   and seeme's.  
   */

#include "jimk.h"
#include "flicmenu.h"
#include "quickdat.str"

extern inverse_cursor(), black_block(), see_islidepot(),
	see_number_slider(),  move_tab_text(), see_ink0(), toggle_mask(),
	change_pen_mode(), ccolor_box(), change_ink_mode(),
	qmake_frames(), go_time_menu(),
	mrewind(), mfast_forward(),wcursor(),draw_quick_menu(),
	mundo_pic(), move_menu(), fill_inkwell(),
	show_sel_mode(), toggle_sel_mode(), see_menu_back(), gary_menu_back(),
	see_range(), see_colors2(), see_ink(),
	pget_color(), ccolor_box(), change_mode(),
	mplayit(), mprev_frame(),mnext_frame(), mfirst_frame(), mlast_frame(),
	text_lineunder(), see_pen(), toggle_pen(), set_pbrush(),
	set_pspeed(), gbnumber_plus1(),
	see_draw_mode(), toggle_draw_mode(), toggle_zoom(),
	bottom_menu(), ppalette(), options(),text_boxp1(),
	white_frame(), white_block(), hang_child(), go_dmmenu(),
	dcorner_text(), ccorner_text(), gary_menu_back(), bcursor(),
	blacktext(), greytext(), grey_block(), toggle_group(),
	set_zoom_level(), see_mask_m(),
	go_multi(), mgo_stencil(), toggle_stencil(), feel_crb(), see_crb(),
	bwtext(), wbtexty1(), wbnumber(), bwnumber(), white_slice();

extern struct cursor cdown, cleft, cright, csleft,
	cinsert, cappend, cright2, cleft2, csright, cup, size_cursor,
	default_c;

extern Flicmenu palette_menu;

static Flicmenu easep_sel = {
	NONEXT,
	NOCHILD,
	306, 189, 10, 8,
	quickdat_100 /* "K" */,
	ccorner_text,
	toggle_group,
	&vs.zero_clear, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu uses_sel = {
	&easep_sel,
	NOCHILD,
	293, 189, 10, 8,
	NULL,
	see_mask_m,
	toggle_mask,
	&vs.use_mask, 1,
	NOKEY,
	mgo_stencil,
	};
static Flicmenu multim_sel = {
	&uses_sel,
	NOCHILD,
	306, 179, 10, 8,
	quickdat_102 /* "T" */,
	ccorner_text,
	toggle_group,
	&vs.multi, 1,
	NOKEY,
	go_multi,
	};
static Flicmenu fillp_sel = {
	&multim_sel,
	NOCHILD,
	293, 179, 10, 8,
	quickdat_103 /* "F" */,
	ccorner_text,
	toggle_group,
	&vs.fillp, 1,
	NOKEY,
	NOOPT,
	};
Flicmenu ccolor_sel = {
	&fillp_sel,
	NOCHILD,
	306, 168, 10, 8,
	NOTEXT,
	ccolor_box,
	ppalette,
	NOGROUP, 0,
	NOKEY,
	ppalette,
	};
static Flicmenu cbrush_sel = {
	&ccolor_sel,
	NOCHILD,
	293, 167, 10, 10,
	NOTEXT,
	see_pen,
	toggle_pen,
	NOGROUP, 0,
	NOKEY,
	set_pbrush,
	};

static Flicmenu it5_sel = {
	NONEXT,
	NOCHILD,
	251, 188, 38, 8,
	NOTEXT,  /* filled in by attatch inks */
	dcorner_text,
	change_ink_mode,
	&vs.draw_mode, 1,
	NOKEY,
	go_dmmenu,
	};
static Flicmenu it4_sel = {
	&it5_sel,
	NOCHILD,
	251, 178, 38, 8,
	NOTEXT,  /* filled in by attatch inks */
	dcorner_text,
	change_ink_mode,
	&vs.draw_mode, 12,
	NOKEY,
	go_dmmenu,
	};
static Flicmenu it3_sel = {
	&it4_sel,
	NOCHILD,
	251, 168, 38, 8,
	NOTEXT,  /* filled in by attatch inks */
	dcorner_text,
	change_ink_mode,
	&vs.draw_mode, 7,
	NOKEY,
	go_dmmenu,
	};
static Flicmenu it2_sel = {
	&it3_sel,
	NOCHILD,
	211, 188, 38, 8,
	NOTEXT,  /* filled in by attatch inks */
	dcorner_text,
	change_ink_mode,
	&vs.draw_mode, 5,
	NOKEY,
	go_dmmenu,
	};
static Flicmenu it1_sel = {
	&it2_sel,
	NOCHILD,
	211, 178, 38, 8,
	NOTEXT,  /* filled in by attatch inks */
	dcorner_text,
	change_ink_mode,
	&vs.draw_mode, 6,
	NOKEY,
	go_dmmenu,
	};
Flicmenu it0_sel = {
	&it1_sel,
	NOCHILD,
	211, 168, 38, 8,
	NOTEXT,  /* filled in by attatch inks */
	dcorner_text,
	change_ink_mode,
	&vs.draw_mode, 0,
	NOKEY,
	go_dmmenu,
	};
Flicmenu itgroup_sel = {
	NONEXT,
	&it0_sel,
	211, 168, 78, 28,
	NULL,
	NOSEE,
	NOFEEL,
	NOGROUP, 0, 
	NOKEY,
	NOOPT,
	};
static Flicmenu ithanger_sel = {
	&cbrush_sel,
	&itgroup_sel,
	211, 168, 78, 28,
	NULL,
	hang_child,
	NOFEEL,
	NOGROUP, 0, 
	NOKEY,
	NOOPT,
	};
static Flicmenu bot_sel = {
	NONEXT,
	NOCHILD,
	197, 189, 8, 8,
	&cdown,
	bcursor,
	mlast_frame,
	NOGROUP, 0,
	NOKEY,
	go_time_menu,
	};

static Flicmenu play_sel = {
	&bot_sel,
	NOCHILD,
	186, 189, 10, 8,
	&cright2,
	bcursor,
	mplayit,
	NOGROUP, 0,
	DARROW,
	go_time_menu,
	};

static Flicmenu fore1_sel = {
	&play_sel,
	NOCHILD,
	175, 189, 9, 8,
	&cright,
	bcursor,
	mnext_frame,
	NOGROUP, 0,
	RARROW,
	go_time_menu,
	};

static Flicmenu frameix_sel = {
	&fore1_sel,
	NOCHILD,
	147, 190, 26, 6,
	&vs.frame_ix,
	gbnumber_plus1,
	go_time_menu,
	NOGROUP, 0,
	NOKEY,
	go_time_menu,
	};

static Flicmenu back1_sel = {
	&frameix_sel,
	NOCHILD,
	136, 189, 9, 8,
	&cleft,
	bcursor,
	mprev_frame,
	NOGROUP, 0,
	LARROW,
	go_time_menu,
	};
static Flicmenu top_sel = {
	&back1_sel,
	NOCHILD,
	127, 189, 8, 8,
	&cup,
	bcursor,
	mfirst_frame,
	NOGROUP, 0,
	UARROW,
	go_time_menu,
	};
Flicmenu minitime_sel = {
	NONEXT,
	&top_sel,
	130, 189, 78, 8,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	go_time_menu,
	};
static Flicmenu minitime_hanger_sel = {
	&ithanger_sel,
	&minitime_sel,
	130, 189, 78, 8,
	NOTEXT,
	hang_child,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
Flicmenu spec1_sel = {
	&minitime_hanger_sel,
	NOCHILD,
	124, 178, 84, 9,
	&vs.crange,
	see_crb,
	feel_crb,
	NOGROUP, 0,
	NOKEY,
	ppalette,
	};
static Flicmenu ink7_sel = {
	NONEXT,
	NOCHILD,
	198, 167, 10, 9,
	NULL,
	see_ink,
	pget_color,
	NOGROUP, 7,
	NOKEY,
	fill_inkwell,
	};
static Flicmenu ink6_sel = {
	&ink7_sel,
	NOCHILD,
	188, 167, 10, 9,
	NULL,
	see_ink,
	pget_color,
	NOGROUP, 6,
	NOKEY,
	fill_inkwell,
	};
static Flicmenu ink5_sel = {
	&ink6_sel,
	NOCHILD,
	178, 167, 10, 9,
	NULL,
	see_ink,
	pget_color,
	NOGROUP, 5,
	NOKEY,
	fill_inkwell,
	};
static Flicmenu ink4_sel = {
	&ink5_sel,
	NOCHILD,
	168, 167, 10, 9,
	NULL,
	see_ink,
	pget_color,
	NOGROUP, 4,
	NOKEY,
	fill_inkwell,
	};
static Flicmenu ink3_sel = {
	&ink4_sel,
	NOCHILD,
	158, 167, 10, 9,
	NULL,
	see_ink,
	pget_color,
	NOGROUP, 3,
	NOKEY,
	fill_inkwell,
	};
static Flicmenu ink2_sel = {
	&ink3_sel,
	NOCHILD,
	148, 167, 10, 9,
	NULL,
	see_ink,
	pget_color,
	NOGROUP, 2,
	NOKEY,
	fill_inkwell,
	};
static Flicmenu ink1_sel = {
	&ink2_sel,
	NOCHILD,
	138, 167, 10, 9,
	NULL,
	see_ink,
	pget_color,
	NOGROUP, 1,
	NOKEY,
	fill_inkwell,
	};
static Flicmenu inks_sel = {
	NONEXT,
	&ink1_sel,
	138, 167, 7*10, 9,
	NULL,
	grey_block,
	NOFEEL,
	NOGROUP, 0, 
	NOKEY,
	NOOPT,
	};
static Flicmenu ink0_sel = {
	&inks_sel,
	NOCHILD,
	124, 167, 10, 9,
	NULL,
	see_ink0,
	pget_color,
	NOGROUP, 0,
	NOKEY,
	fill_inkwell,
	};
Flicmenu ink_group_sel = {
	NONEXT,
	&ink0_sel,
	124, 167, 85, 9,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu ink_hanger_sel = {
	&spec1_sel,
	&ink_group_sel,
	124, 167, 85, 9,
	NOTEXT,
	hang_child,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu dsel6_sel = {
	NONEXT,
	NOCHILD,
	83, 188, 38, 8,
	NOTEXT,
	dcorner_text,
	change_pen_mode,
	&vs.tool_ix, 5,
	NOKEY,
	options,
	};
static Flicmenu dsel5_sel = {
	&dsel6_sel,
	NOCHILD,
	83, 178, 38, 8,
	NOTEXT,
	dcorner_text,
	change_pen_mode,
	&vs.tool_ix, 4,
	NOKEY,
	options,
	};
static Flicmenu dsel4_sel = {
	&dsel5_sel,
	NOCHILD,
	83, 168, 38, 8,
	NOTEXT,
	dcorner_text,
	change_pen_mode,
	&vs.tool_ix, 3,
	NOKEY,
	options,
	};
static Flicmenu dsel3_sel = {
	&dsel4_sel,
	NOCHILD,
	43, 188, 38, 8,
	NOTEXT,
	dcorner_text,
	change_pen_mode,
	&vs.tool_ix, 2,
	NOKEY,
	options,
	};
static Flicmenu dsel2_sel = {
	&dsel3_sel,
	NOCHILD,
	43, 178, 38, 8,
	NOTEXT,
	dcorner_text,
	change_pen_mode,
	&vs.tool_ix, 1,
	NOKEY,
	options,
	};
Flicmenu dsel1_sel = {
	&dsel2_sel,
	NOCHILD,
	43, 168, 38, 8,
	NOTEXT,
	dcorner_text,
	change_pen_mode,
	&vs.tool_ix, 0,
	NOKEY,
	options,
	};
Flicmenu dsel_group_sel = {
	NONEXT,
	&dsel1_sel,
	43, 168, 78, 28,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
Flicmenu dsel_hanger_sel = {
	&ink_hanger_sel,
	&dsel_group_sel,
	43, 168, 78, 28,
	NOTEXT,
	hang_child,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

static Flicmenu undo_sel = {
	&dsel_hanger_sel,
	NOCHILD,
	4, 188, 34, 8,
	quickdat_104 /* "UNDO" */,
	ccorner_text,
	mundo_pic,
	NOGROUP, 0,
	'\b',
	NOOPT,
	};
static Flicmenu zoom_sel = {
	&undo_sel,
	NOCHILD,
	4, 178, 34, 8,
	quickdat_105 /* "ZOOM" */,
	ccorner_text,
	toggle_zoom,
	&vs.zoom_mode, 1,
	'z',
	set_zoom_level,
	};
static Flicmenu moveq_sel = {
	&zoom_sel,
	NOCHILD,
	3,168,36,8,
	quickdat_106 /* "HOME" */,
	move_tab_text,
	move_menu,
	NOGROUP, 0,
	NOKEY,
	bottom_menu,
	};
Flicmenu quick_menu = 
	{
	NONEXT,
	&moveq_sel,
	0, 165,	319, 34,
	NOTEXT,
	gary_menu_back,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};


static
see_ink(m)
Flicmenu *m;
{
WORD color;

color = vs.inks[m->identity];
color_block1(color,m);
if (color == vs.ccolor)
	a_frame(sred, m);
}

static
see_ink0(m)
Flicmenu *m;
{
a_frame(sgrey, m);
see_ink(m);
}

see_pen(m)
Flicmenu *m;
{
cdraw_brush(dot_pens[vs.pen_width], m->x+((m->width)>>1), 
	m->y+((m->height)>>1), sgrey);
}

/* redraw selections after changing current color */
predraw()
{
if (cur_menu == &palette_menu)
	{
	redraw_ccolor();
	}
else
	{
	qdraw_a_menu(&ink_group_sel);
	draw_sel(&ccolor_sel);
	draw_sel(&spec1_sel);
	}
}

static
show_ink(c)
int c;
{
char buf[40];
UBYTE *m;

m = render_form->cmap + 3*c;
sprintf(buf, quickdat_107 /* "color %d rgb %d %d %d" */, c, m[0], m[1], m[2]);
top_text(buf);
}


static
fill_inkwell(m)
Flicmenu *m;
{
int c, oc;

a_frame(sred, m);
if ((c = get_a_end(show_ink))>=0)
	{
	vs.inks[m->identity] = c;
	}
predraw();
}

static
pget(m)
Flicmenu *m;
{
if ( uzx >= m->x+1 && uzx < m->x + m->width &&
	uzy >= m->y +1 && uzy < m->y + m->height)
	{
	vs.ccolor = getdot(uzx, uzy);
	vs.cycle_draw = 0;
	}
}



static
pget_color(m)
Flicmenu *m;
{
pget(m);
predraw();
}


static
see_crb(m)
Flicmenu *m;
{
m->identity = vs.use_bun;
see_cluster(m);
}

static
feel_crb(m)
Flicmenu *m;
{
m->identity = vs.use_bun;
feel_cluster(m);
}

toggle_mask(m)
Flicmenu *m;
{
quse_mask();
draw_sel(m);
}

see_mask_m(m)
Flicmenu *m;
{
m->disabled = (mask_plane == NULL);
m->text = quickdat_101;
ccorner_text(m);
}

