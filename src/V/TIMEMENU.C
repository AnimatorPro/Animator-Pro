
/* timemenu.c - The data structures and some associated code for the
   Frames control panel. */

#include "jimk.h"
#include "flicmenu.h"
#include "fli.h"
#include "timemenu.str"

extern dcorner_text(), ccorner_text(), ncorner_text(), gary_menu_back(),
	ccolor_box(), ppalette(), see_pen(), toggle_pen(), set_pbrush(),
	ccorner_cursor(), blacktext(), greytext(), toggle_group(),
	insert_a_frame(), kill_a_frame(), set_total_frames(),
	see_time_slider(), feel_time_slider(), see_range_width(),
	see_qslider(), feel_qslider(), ncorner_number(),
	mplayit(), mfirst_frame(), mlast_frame(), minsert(), mdelete_frame(),
	jump_to_frame(), jump_to_mark(), set_mark(),
	frame_double(), go_multi(), bcursor(),
	mplayit(), mprev_frame(),mnext_frame(), mfirst_frame(), mlast_frame(),
	see_range_button(), use_range_button(), set_range_button(),
	move_tab_text(), move_menu(), bottom_menu(),
	multi_preview(), multi_use(), close_menu(),
	hang_child(), toggle_group(), change_mode();

#define TR1 155
#define TR2 187

extern Flicmenu minitime_sel, tseg_group_sel, quick_menu;

extern struct cursor cdown, cleft, cright, csleft,
	cinsert, cappend, cright2, cleft2, csright, cup, size_cursor,
	default_c, ckill;

static struct qslider frame_sl = {0, 0, &vs.frame_ix, 1, NULL};
static struct qslider speed_sl = { 0, 120, &fhead.speed, 0, };


static Flicmenu tmu_t5_sel = {
	NONEXT,
	NOCHILD,
	253, 155, 16, 10,
	timemenu_100 /* "*5" */,
	ccorner_text,
	frame_double,
	NOGROUP, 5,
	NOKEY,
	NOOPT,
	};
static Flicmenu tmu_t3_sel = {
	&tmu_t5_sel,
	NOCHILD,
	234, 155, 16, 10,
	timemenu_101 /* "*3" */,
	ccorner_text,
	frame_double,
	NOGROUP, 3,
	NOKEY,
	NOOPT,
	};
static Flicmenu tmu_t2_sel = {
	&tmu_t3_sel,
	NOCHILD,
	215, 155, 16, 10,
	timemenu_102 /* "*2" */,
	ccorner_text,
	frame_double,
	NOGROUP, 2,
	NOKEY,
	NOOPT,
	};
static Flicmenu tmu_md_sel = {
	&tmu_t2_sel,
	NOCHILD,
	194+1, 155, 12, 10,
	timemenu_108 /* "D" */,
	ccorner_text,
	jump_to_mark,
	&vs.frame_ix, 1000-1,
	NOKEY,
	set_mark,
	};
static Flicmenu tmu_mc_sel = {
	&tmu_md_sel,
	NOCHILD,
	179+1, 155, 12, 10,
	timemenu_109 /* "C" */,
	ccorner_text,
	jump_to_mark,
	&vs.frame_ix, 100-1,
	NOKEY,
	set_mark,
	};
static Flicmenu tmu_mb_sel = {
	&tmu_mc_sel,
	NOCHILD,
	164+1, 155, 12, 10,
	timemenu_110 /* "B" */,
	ccorner_text,
	jump_to_mark,
	&vs.frame_ix, 10-1,
	NOKEY,
	set_mark,
	};
static Flicmenu tmu_ma_sel = {
	&tmu_mb_sel,
	NOCHILD,
	149+1, 155, 12, 10,
	timemenu_111 /* "A" */,
	ccorner_text,
	jump_to_mark,
	&vs.frame_ix, 0,
	NOKEY,
	set_mark,
	};
static Flicmenu tmu_marks_sel = {
	&tmu_ma_sel,
	NOCHILD,
	111+3, 155, 32, 8,
	timemenu_107 /* "MARKS" */,
	blacktext,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tmu_rd_sel = {
	&tmu_marks_sel,
	NOCHILD,
	92+4, 155, 12, 10,
	timemenu_108 /* "D" */,
	see_range_button,
	use_range_button,
	NOGROUP, 3,
	NOKEY,
	set_range_button,
	};
static Flicmenu tmu_rc_sel = {
	&tmu_rd_sel,
	NOCHILD,
	77+4, 155, 12, 10,
	timemenu_109 /* "C" */,
	see_range_button,
	use_range_button,
	NOGROUP, 2,
	NOKEY,
	set_range_button,
	};
static Flicmenu tmu_rb_sel = {
	&tmu_rc_sel,
	NOCHILD,
	62+4, 155, 12, 10,
	timemenu_110 /* "B" */,
	see_range_button,
	use_range_button,
	NOGROUP, 1,
	NOKEY,
	set_range_button,
	};
static Flicmenu tmu_ra_sel = {
	&tmu_rb_sel,
	NOCHILD,
	47+4, 155, 12, 10,
	timemenu_111 /* "A" */,
	see_range_button,
	use_range_button,
	NOGROUP, 0,
	NOKEY,
	set_range_button,
	};
static Flicmenu tmu_ranges_sel = {
	&tmu_ra_sel,
	NOCHILD,
	6, 155, 38, 8,
	timemenu_112 /* "SEGMENT" */,
	blacktext,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tmu_tseg_hanger = {
	&tmu_ranges_sel,
	&tseg_group_sel,
	4, 170, 311, 10,
	NOTEXT,
	hang_child,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tmu_playr_sel = {
	&tmu_tseg_hanger,
	NOCHILD,
	228,185,81,10,
	timemenu_113 /* "TIME SELECT" */,
	dcorner_text,
	toggle_group,
	&vs.multi, 1,
	NOKEY,
	go_multi,
	};
static Flicmenu tmu_spdsl_sel = {
	&tmu_playr_sel,
	NOCHILD,
	70,185,147,10,
	&speed_sl,
	see_qslider,
	feel_qslider,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tmu_spdtag_sel = {
	&tmu_spdsl_sel,
	NOCHILD,
	3,186,64,8,
	timemenu_114 /* "PLAY SPEED" */,
	blacktext,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tmu_frame_ct_sel = {
	&tmu_spdtag_sel,
	NOCHILD,
	242,141,27,10,
	&fhead.frame_count,
	ncorner_number,
	set_total_frames,
	NOGROUP, 0,
	NOKEY,
	set_total_frames,
	};
static Flicmenu tmu_frame_sl_sel = {
	&tmu_frame_ct_sel,
	NOCHILD,
	60,141,156,10,
	&frame_sl,
	see_time_slider,
	feel_time_slider,
	NOGROUP, 0,
	NOKEY,
	jump_to_frame,
	};
static Flicmenu tmu_play_sel = {
	&tmu_frame_sl_sel,
	NOCHILD,
	218,141,12,10,
	&cright2,
	bcursor,
	mplayit,
	NOGROUP, 0,
	DARROW,
	NOOPT,
	};
static Flicmenu tmu_down_sel = {
	&tmu_play_sel,
	NOCHILD,
	230,141,10,10,
	&cdown,
	bcursor,
	mlast_frame,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tmu_up_sel = {
	&tmu_down_sel,
	NOCHILD,
	49,141,12,10,
	&cup,
	bcursor,
	mfirst_frame,
	NOGROUP, 0,
	UARROW,
	NOOPT,
	};
static Flicmenu tmu_kill_sel = {
	&tmu_up_sel,
	NOCHILD,
	274,155,41,10,
	timemenu_115 /* "delete" */,
	ccorner_text,
	kill_a_frame,
	NOGROUP, 0,
	DELKEY,
	mdelete_frame,
	};
static Flicmenu tmu_insert_sel = {
	&tmu_kill_sel,
	NOCHILD,
	274,141,41,10,
	timemenu_116 /* "insert" */,
	ccorner_text,
	insert_a_frame,
	NOGROUP, 0,
	INSERTKEY,
	minsert,
	};
static Flicmenu tmu_nx_sel = {
	&tmu_insert_sel,
	NOCHILD,
	253, 155, 0, 0,
	NOTEXT,
	NOSEE,
	mnext_frame,
	NOGROUP, 0,
	RARROW,
	NOOPT,
	};
static Flicmenu tmu_pv_sel = {
	&tmu_nx_sel,
	NOCHILD,
	253, 155, 0, 0,
	NOTEXT,
	NOSEE,
	mprev_frame,
	NOGROUP, 0,
	LARROW,
	NOOPT,
	};
static Flicmenu tmu_moveq_sel = {
	&tmu_pv_sel,
	NOCHILD,
	4,140,42,10,
	timemenu_117 /* "FRAMES" */,
	move_tab_text,
	move_menu,
	NOGROUP, 0,
	NOKEY,
	bottom_menu,
	};
Flicmenu tmu_menu = 
	{
	NONEXT,
	&tmu_moveq_sel,
	0, 137, 319, 62,
	NOTEXT,
	gary_menu_back,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};


static
feel_time_slider(m)
Flicmenu *m;
{
int oframe_ix;
int newix;

oframe_ix = vs.frame_ix;
feel_qslider(m);
newix = vs.frame_ix;
vs.frame_ix = oframe_ix;
hide_mp();
scrub_cur_frame();
copy_form(render_form, &uf);
vs.frame_ix = newix;
fli_tseek(&uf, oframe_ix, vs.frame_ix);
copy_form(&uf, render_form);
see_cmap();
zoom_it();
draw_mp();
}

static
jump_to_frame(m)
Flicmenu *m;
{
int x;

if (!tflx)
	return;
hide_mp();
if (in_right_arrow(m))
	{
	x = 1;
	if (qreq_number(timemenu_118 /* "Move forward how many frames?" */, 
		&x, 1, fhead.frame_count))
		x = vs.frame_ix + x;
	else
		goto OUT;
	}
else if (in_left_arrow(m))
	{
	x = 1;
	if (qreq_number(timemenu_119 /* "Move back how many frames?" */, 
		&x, 1, fhead.frame_count))
		x = vs.frame_ix - x;
	else
		goto OUT;
	}
else
	{
	x = vs.frame_ix+1;
	if (qreq_number(timemenu_120 /* "Jump to frame number?" */, 
		&x, 1, fhead.frame_count))
		x -= 1;
	else
		goto OUT;
	}
x = wrap_frame(x);
scrub_cur_frame();
save_undo();
fli_tseek(&uf, vs.frame_ix, x);
vs.frame_ix = x;
jset_colors(0, COLORS, uf.cmap);
copy_form(&uf, render_form);
zoom_it();
OUT:
draw_mp();
}


static
see_time_slider(m)
Flicmenu *m;
{
frame_sl.max = fhead.frame_count-1;
see_qtslider(m);
}

static
insert_some(frames)
int frames;
{
hide_mp();
unzoom();
scrub_cur_frame();
insert_frames(frames, vs.frame_ix);
rezoom();
draw_mp();
}

insert_a_frame()
{
insert_some(1);
}

static
minsert()
{
hide_mp();
unzoom();
qinsert_frames();
rezoom();
draw_mp();
}

static
mdelete_frame()
{
hide_mp();
unzoom();
qdelete_frames();
rezoom();
draw_mp();
}

kill_a_frame()
{
hide_mp();
unzoom();
if (yes_no_line(timemenu_121 /* "Delete this frame?" */))
	delete_some(1);
rezoom();
draw_mp();
}

static
set_total_frames()
{
hide_mp();
unzoom();
qmake_frames();
rezoom();
draw_mp();
}

static
set_range_button(m)
Flicmenu *m;
{
int ix;

ix = m->identity;
vs.starttr[ix] = vs.start_seg;
vs.stoptr[ix] = vs.stop_seg;
redraw_range_buttons();
}

static
see_range_button(m)
Flicmenu *m;
{
int ix;

ix = m->identity;
if (vs.start_seg == vs.starttr[ix] && vs.stop_seg == vs.stoptr[ix])
	{
	m->group = &ix;
	}
else
	{
	m->group = NULL;
	}
ccorner_text(m);
}

redraw_range_buttons()
{
draw_sel(&tmu_ra_sel);
draw_sel(&tmu_rb_sel);
draw_sel(&tmu_rc_sel);
draw_sel(&tmu_rd_sel);
}

static
use_range_button(m)
Flicmenu *m;
{
int ix;

ix = m->identity;
vs.start_seg = vs.starttr[ix];
vs.stop_seg = vs.stoptr[ix];
clip_tseg();
redraw_tseg();
redraw_range_buttons();
}


static
set_mark(m)
Flicmenu *m;
{
int ix;

ix = el_ix((Name_list *)&tmu_ma_sel, (Name_list *)m);
m->identity = vs.marks[ix] = vs.frame_ix;
change_mode(m);
}

static
jump_to_mark(m)
Flicmenu *m;
{
int ix;

ix = m->identity;
if (ix >= fhead.frame_count)
	ix = fhead.frame_count-1;
hide_mp();
unzoom();
scrub_cur_frame();
save_undo();
fli_tseek(&uf, vs.frame_ix, ix);
vs.frame_ix = ix;
copy_form(&uf, render_form);
see_cmap();
rezoom();
draw_mp();
}

static
attatch_marks()
{
WORD *mark;
Flicmenu *menu;
int i;

i = 4;
menu = &tmu_ma_sel;
mark = vs.marks;
while (--i >= 0)
	{
	menu->identity = *mark++;
	menu = menu->next;
	}
}

extern check_max_frames(long new);

static
frame_double(m)
Flicmenu *m;
{
long i;
long count;
int mult;

hide_mp();
unzoom();
scrub_cur_frame();
count = fhead.frame_count;
mult = m->identity;
if (check_max_frames(count*mult))
	{
	for (i=0; i<count; i++)
		{
		if (!insert_frames(mult-1, i*mult))
			break;
		}
	vs.frame_ix *= mult;
	vs.bframe_ix *= mult;
	}
rezoom();
draw_mp();
}

do_time_menu()
{
int i;
static int in_tm = 0;

if (!in_tm)
	{
	in_tm = 1;
	clip_tseg();
	attatch_marks();
	clip_rmove_menu(&tmu_menu, 
		quick_menu.x - tmu_menu.x, quick_menu.y-tmu_menu.y); 
	do_menu(&tmu_menu);
	in_tm = 0;
	}
else
	{
	go_in_circles_message(timemenu_122 /* "frames" */);
	}
}

go_time_menu()
{
hide_mp();
do_time_menu();
draw_mp();
}
