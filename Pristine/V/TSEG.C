
/* tseg.c - data structures and associated code to implement "Time Segment
   selector" */

#include "jimk.h"
#include "flicmenu.h"
#include "fli.h"
#include "commonst.h"
#include "tseg.str"

extern dcorner_text(), ccorner_text(), gary_menu_back(), mundo_pic(),
	toggle_pen(), palette(), ccolor_box(), ncorner_int(),
	ccorner_cursor(), ncorner_cursor(), ncorner_text(),
	ncorner_number(), see_seg_size(), see_tseg(), feel_tseg(), rfeel_tseg(),
	hang_child(), move_tab_text(), move_menu(), bottom_menu(),
	stuff_with_cur_ix(), see_twidth(), change_mode(), change_time_mode(),
	see_pen(), toggle_group(), set_pbrush(); 

extern inc_tseg(), dec_tseg();

extern Flicmenu minitime_sel;

extern struct cursor cdown, cleft, cright, csleft,
	cinsert, cappend, cright2, cleft2, csright, cup, size_cursor,
	default_c;

#define OX -4
#define OY 5


static Flicmenu ts_wid_sel = {
	NONEXT,
	NOCHILD,
	OX+289, OY+144, 26, 10,
	0,
	see_twidth,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu ts_a_sel = {
	&ts_wid_sel,
	NOCHILD,
	OX+274, OY+145, 10, 8,
	tseg_100 /* "a" */,
	ccorner_text,
	change_time_mode,
	&vs.time_mode,2,
	NOKEY,
	NOOPT,
	};
static Flicmenu ts_s_sel = {
	&ts_a_sel,
	NOCHILD,
	OX+264, OY+145, 10, 8,
	tseg_101 /* "s" */,
	ccorner_text,
	change_time_mode,
	&vs.time_mode,1,
	NOKEY,
	NOOPT,
	};
static Flicmenu ts_f_sel = {
	&ts_s_sel,
	NOCHILD,
	OX+254, OY+145, 10, 8,
	tseg_102 /* "f" */,
	ccorner_text,
	change_time_mode,
	&vs.time_mode,0,
	NOKEY,
	NOOPT,
	};

static Flicmenu tsg_rs3_sel = {
	&ts_f_sel,
	NOCHILD,
	234, 149, 11, 10,
	&cright,
	ncorner_cursor,
	inc_tseg,
	&vs.stop_seg, -1,
	NOKEY,
	NOOPT,
	};
static Flicmenu tsg_rs2_sel = {
	&tsg_rs3_sel,
	NOCHILD,
	208, 149, 26, 10,
	&vs.stop_seg,
	ncorner_int,
	stuff_with_cur_ix,
	&vs.stop_seg, -1,
	NOKEY,
	NOOPT,
	};
static Flicmenu tsg_rs1_sel = {
	&tsg_rs2_sel,
	NOCHILD,
	197, 149, 11, 10,
	&cleft,
	ncorner_cursor,
	dec_tseg,
	&vs.stop_seg, -1,
	NOKEY,
	NOOPT,
	};
static Flicmenu tsg_tseg_sel = {
	&tsg_rs1_sel,
	NOCHILD,
	48, 149, 149, 10,
	NOTEXT,
	see_tseg,
	feel_tseg,
	&vs.start_seg, -1,
	NOKEY,
	rfeel_tseg,
	};
static Flicmenu tsg_ls3_sel = {
	&tsg_tseg_sel,
	NOCHILD,
	37, 149, 11, 10,
	&cright,
	ncorner_cursor,
	inc_tseg,
	&vs.start_seg, -1,
	NOKEY,
	NOOPT,
	};
static Flicmenu tsg_ls2_sel = {
	&tsg_ls3_sel,
	NOCHILD,
	11, 149, 26, 10,
	&vs.start_seg,
	ncorner_int,
	stuff_with_cur_ix,
	&vs.start_seg, -1,
	NOKEY,
	NOOPT,
	};
static Flicmenu tsg_ls1_sel = {
	&tsg_ls2_sel,
	NOCHILD,
	0, 149, 11, 10,
	&cleft,
	ncorner_cursor,
	dec_tseg,
	&vs.start_seg, -1,
	NOKEY,
	NOOPT,
	};
Flicmenu tseg_group_sel = {
	NONEXT,
	&tsg_ls1_sel,
	0, 149, 319, 10,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

extern inc_tseg(), dec_tseg(), inc_tseg(), dec_tseg();

static WORD *itg;

static
see_twidth(m)
Flicmenu *m;
{
extern int tr_frames;

find_range();
m->text = &tr_frames;
ncorner_number(m);
m->text = NULL;
}

redraw_tseg()
{
extern Flicmenu tmu_menu;

draw_sel(&tsg_rs2_sel);
draw_sel(&tsg_ls2_sel);
draw_sel(&tsg_tseg_sel);
draw_sel(&ts_wid_sel);
if (cur_menu == &tmu_menu)	/* in time menu? Update ranges... */
	redraw_range_buttons();
}

static
itseg()
{
if (*itg < fhead.frame_count-1)
	{
	*itg += 1;
	redraw_tseg();
	}
}

static
dtseg()
{
if (*itg > 0)
	{
	*itg -= 1;
	redraw_tseg();
	}
}

static
inc_tseg(m)
Flicmenu *m;
{
hilight(m);
itg = m->group;
repeat_on_pdn(itseg);
draw_sel(m);
}

static
dec_tseg(m)
Flicmenu *m;
{
hilight(m);
itg = m->group;
repeat_on_pdn(dtseg);
draw_sel(m);
}

static int seg_size;
static int seg_lower;
static char seg_sign;

static
find_seg_size()
{
seg_size = vs.stop_seg - vs.start_seg;
if (seg_size < 0)
	{
	seg_sign = 1;
	seg_size = -seg_size;
	seg_lower = vs.stop_seg;
	}
else
	{
	seg_sign = 0;
	seg_lower = vs.start_seg;
	}
seg_size += 1;
}

#ifdef SLUFFED
see_seg_size(m)
Flicmenu *m;
{
char buf[40];
int ssize;

find_seg_size();
sprintf(buf, "%s%d of %d", 
	(seg_sign ? "-" : cst_), seg_size, (int)fhead.frame_count);
m->text = buf;
mb_inside(m,swhite);
ncorner_text(m);
}
#endif SLUFFED

static int arrow_width;
static int arrow_start;
static int nwidth, nx;
static int asteps;

static
see_tseg(m)
Flicmenu *m;
{
int i;
int ny, nh;
int stepinc;
int astart;

clip_tseg();
find_seg_size();
nh = m->height - 4;
asteps = (nh>>1);
nwidth = m->width - 4 - asteps;
nx = m->x + 2 + asteps;
ny = m->y + 2;
m2color_block(m, sgrey, swhite);
arrow_width = rscale_by(nwidth, seg_size, fhead.frame_count)+1;
astart = arrow_start = rscale_by(nwidth, seg_lower, fhead.frame_count) + nx;
if (seg_sign)
	{
	stepinc = -1;
	}
else
	{
	stepinc = 1;
	astart -= asteps;
	}
if (seg_size == 1)
	{
	stepinc = 0;
	}
for (i=0; i<=nh; i++)
	{
	chli(vf.p, astart, ny++, arrow_width, sblack);
	if (i == asteps)
		{
		stepinc = -stepinc;
		}
	astart+=stepinc;
	}
}

static
prop_clip_tseg()
{
int x, dx;

if (vs.start_seg < 0)
	{
	vs.stop_seg -= vs.start_seg;
	vs.start_seg = 0;
	}
if (vs.stop_seg < 0)
	{
	vs.start_seg -= vs.stop_seg;
	vs.stop_seg = 0;
	}
if ((dx = fhead.frame_count - 1 - vs.start_seg) < 0)
	{
	vs.start_seg += dx;
	vs.stop_seg += dx;
	}
if ((dx = fhead.frame_count - 1 - vs.stop_seg) < 0)
	{
	vs.start_seg += dx;
	vs.stop_seg += dx;
	}
}



static
prop_move_tseg(x)
int x;
{
vs.start_seg += x;
vs.stop_seg += x;
prop_clip_tseg();
redraw_tseg();
}

static
feel_tseg(m)
Flicmenu *m;
{
if (uzx < arrow_start)		/* page left */
	{
	prop_move_tseg(-seg_size);
	}
else if (uzx >= arrow_start + arrow_width)   /* page right */
	{
	prop_move_tseg(seg_size);
	}
else		/* drag it around keeping length the same */
	{
	prop_drag_tseg();
	}
}


static
prop_drag_tseg()
{
int lastp, p;
int firstx, dx;


lastp = 0;
firstx = lastx = uzx;
for (;;)
	{
	wait_input();
	if (!PDN)
		break;
	dx = uzx - firstx;
	p = rscale_by(dx, fhead.frame_count, nwidth);
	prop_move_tseg(p - lastp);
	lastp = p;
	}
}

static
tseg_dist(seg_end)
int seg_end;
{
return(intabs(
	rscale_by(seg_end, nwidth, fhead.frame_count) + nx - uzx));
}


static
rfeel_tseg(m)
Flicmenu *m;
{
int *which;		/* start or stop of segment??? */
int dist;


dist = tseg_dist(vs.start_seg);
if (dist < tseg_dist(vs.stop_seg))
	{
	which = &vs.start_seg;
	}
else
	{
	which = &vs.stop_seg;
	}
for (;;)
	{
	wait_input();
	if (!RDN)
		break;
	*which = rscale_by( uzx - nx, fhead.frame_count, nwidth);
	clip_tseg();
	redraw_tseg();
	}
}

change_time_mode(m)
Flicmenu *m;
{
change_mode(m);
draw_sel(&ts_wid_sel);
}


static
stuff_with_cur_ix(m)
Flicmenu *m;
{
int *v;

v = m->text;
*v = vs.frame_ix;
redraw_tseg();
}
