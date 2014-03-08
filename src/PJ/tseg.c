
/* tseg.c - data structures and associated code to implement "Time Segment
   selector" */

#include "jimk.h"
#include "flx.h"
#include "menus.h"

void see_seg_size(), see_tseg(), feel_tseg(), rfeel_tseg(),
	stuff_with_cur_ix(), see_twidth(), change_time_mode();

extern void inc_tseg(), dec_tseg();

extern Image cleft, cright; 

static Button tsg_rs3_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	12,11,234,149,
	&cright,
	wbg_ncorner_image,
	inc_tseg,
	NOOPT,
	&vs.stop_seg,-1,
	NOKEY,
	0
	);
static Button tsg_rs2_sel = MB_INIT1(
	&tsg_rs3_sel,
	NOCHILD,
	27,11,208,149,
	&vs.stop_seg,
	ncorner_short_plus1,
	stuff_with_cur_ix,
	NOOPT,
	&vs.stop_seg,-1,
	NOKEY,
	0
	);
static Button tsg_rs1_sel = MB_INIT1(
	&tsg_rs2_sel,
	NOCHILD,
	12,11,197,149,
	&cleft,
	wbg_ncorner_image,
	dec_tseg,
	NOOPT,
	&vs.stop_seg,-1,
	NOKEY,
	0
	);
static Button tsg_tseg_sel = MB_INIT1(
	&tsg_rs1_sel,
	NOCHILD,
	150,11,48,149,
	NOTEXT,
	see_tseg,
	feel_tseg,
	rfeel_tseg,
	&vs.start_seg,0,
	NOKEY,
	0
	);
static Button tsg_ls3_sel = MB_INIT1(
	&tsg_tseg_sel,
	NOCHILD,
	12,11,37,149,
	&cright,
	wbg_ncorner_image,
	inc_tseg,
	NOOPT,
	&vs.start_seg,0,
	NOKEY,
	0
	);
static Button tsg_ls2_sel = MB_INIT1(
	&tsg_ls3_sel,
	NOCHILD,
	27,11,11,149,
	&vs.start_seg,
	ncorner_short_plus1,
	stuff_with_cur_ix,
	NOOPT,
	&vs.start_seg,0,
	NOKEY,
	0
	);
Button tseg_slider_sel = MB_INIT1(
	&tsg_ls2_sel,
	NOCHILD,
	12,11,0,149,
	&cleft,
	wbg_ncorner_image,
	dec_tseg,
	NOOPT,
	&vs.start_seg,0,
	NOKEY,
	0
	);


static Button ts_wid_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	26+1,11,285,149,
	0,
	see_twidth,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
Button tseg_a_sel = MB_INIT1(
	&ts_wid_sel,
	NOCHILD,
	11,9,270,150,
	NODATA, /* "A", */ /* loaded by init_menu_parts() */
	dcorner_text,
	change_time_mode,
	NOOPT,
	&vs.time_mode,2,
	NOKEY,
	MB_GHILITE
	);
Button tseg_s_sel = MB_INIT1(
	&tseg_a_sel,
	NOCHILD,
	11,9,260,150,
	NODATA, /* "S", */ /* loaded by init_menu_parts() */
	dcorner_text,
	change_time_mode,
	NOOPT,
	&vs.time_mode,1,
	NOKEY,
	MB_GHILITE
	);
Button tseg_f_sel = MB_INIT1(
	&tseg_s_sel,
	NOCHILD,
	11,9,250,150,
	NODATA, /* "F", */ /* loaded by init_menu_parts() */
	dcorner_text,
	change_time_mode,
	NOOPT,
	&vs.time_mode,0,
	NOKEY,
	MB_GHILITE
	);

Button tseg_group_sel = MB_INIT1(
	&tseg_f_sel,
	&tseg_slider_sel,
	320,11,0,149,
	NOTEXT,
	hang_children,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);

void inc_tseg(), dec_tseg(), inc_tseg(), dec_tseg(), prop_drag_tseg();

static SHORT *itg;

static void see_twidth(Button *m)
{
extern SHORT tr_frames;

	find_seg_range();
	m->datme = &tr_frames;
	ncorner_short(m);
	m->datme = NULL;
}

void redraw_tseg(Button *b)
{
extern Menuhdr tmu_menu;

	draw_buttontop(&tsg_rs2_sel);
	draw_buttontop(&tsg_ls2_sel);
	draw_buttontop(&tsg_tseg_sel);
	draw_buttontop(&ts_wid_sel);
	if(get_button_hdr(b) == &tmu_menu)	/* in time menu? Update ranges... */
		redraw_range_buttons();
}

static void itseg(Button *b)
{
	if (*itg < flix.hdr.frame_count-1)
	{
		*itg += 1;
		redraw_tseg(b);
	}
}

static void dtseg(Button *b)
{
	if (*itg > 0)
	{
		*itg -= 1;
		redraw_tseg(b);
	}
}

static void inc_tseg(Button *m)
{
	hilight(m);
	itg = m->group;
	repeat_on_pdn(itseg,m);
	draw_buttontop(m);
}

static void dec_tseg(Button *m)
{
	hilight(m);
	itg = m->group;
	repeat_on_pdn(dtseg,m);
	draw_buttontop(m);
}

static int seg_size;
static int seg_lower;
static char seg_sign;
static int arrow_width;
static int arrow_start;
static int nwidth, nx;
static int end_gap;

static void find_seg_size(void)
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

static void see_tseg(Button *b)
{
int i;
int ny, nh;
int stepinc;
int astart;
int minwid;
int asteps;
Wscreen *ws = b->root->w.W_screen;

	clip_tseg();
	find_seg_size();
	nh = b->height - 4;
	asteps = nh/2;
	end_gap = (asteps+1)/2;

	nwidth = b->width - 4 - (end_gap*2);
	nx = b->x + 2 + end_gap;
	ny = b->y + 2;
	m2color_block(b, ws->SGREY, ws->SWHITE);
	arrow_width = rscale_by(nwidth, seg_size, flix.hdr.frame_count);

	/** make sure a minimum ammount of the arrow is present 
	 ** and centered on the spot required */

	minwid = (mb_mscale_x(b,3)>>1);
	if(arrow_width < minwid)
	{
		arrow_width = minwid;
		nx -= (minwid-arrow_width)/2+1;
	}

	arrow_start = astart 
			= rscale_by(nwidth, seg_lower, flix.hdr.frame_count) + nx;

	if (seg_size == 1)
	{
		stepinc = 0;
	}
	else if (seg_sign)
	{
		stepinc = -1;
		astart += end_gap;
	}
	else
	{
		stepinc = 1;
		astart -= end_gap;
	}

	for (i=0; i < nh; ++i)
	{
		pj_set_hline(b->root, ws->SBLACK, astart, ny++, arrow_width);
		if (i == asteps)
		{
			stepinc = -stepinc;
		}
		astart+=stepinc;
	}
}

static void prop_clip_tseg(void)
{
int dx;

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
	if ((dx = flix.hdr.frame_count - 1 - vs.start_seg) < 0)
	{
		vs.start_seg += dx;
		vs.stop_seg += dx;
	}
	if ((dx = flix.hdr.frame_count - 1 - vs.stop_seg) < 0)
	{
		vs.start_seg += dx;
		vs.stop_seg += dx;
	}
}



static void prop_move_tseg(Button *b,int x)
{
vs.start_seg += x;
vs.stop_seg += x;
prop_clip_tseg();
redraw_tseg(b);
}

static void feel_tseg(Button *b)
{
int feel_extra;

	feel_extra = (end_gap+1)/2;
	if (icb.mx < (arrow_start-feel_extra))		/* page left */
	{
		prop_move_tseg(b,-seg_size);
	}
	else if (icb.mx >= (arrow_start+arrow_width+feel_extra))   /* page right */
	{
		prop_move_tseg(b,seg_size);
	}
	else		/* drag it around keeping length the same */
	{
		prop_drag_tseg(b);
	}
}


static void prop_drag_tseg(Button *b)
{
int lastp, p;
int firstx, dx;

	lastp = 0;
	firstx = icb.mx;
	for (;;)
	{
		wait_any_input();
		if (!ISDOWN(MBPEN))
			break;
		dx = icb.mx - firstx;
		p = rscale_by(dx, flix.hdr.frame_count, nwidth);
		prop_move_tseg(b,p - lastp);
		lastp = p;
	}
}

static int tseg_dist(int seg_end)
{
return(intabs(rscale_by(seg_end, nwidth, flix.hdr.frame_count) + nx - icb.mx));
}


static void rfeel_tseg(Button *m)
{
SHORT *which;		/* start or stop of segment??? */
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
	wait_any_input();
	if (!ISDOWN(MBRIGHT))
		break;
	*which = rscale_by( icb.mx - nx, flix.hdr.frame_count, nwidth);
	clip_tseg();
	redraw_tseg(m);
	}
}

void change_time_mode(Button *m)
{
change_mode(m);
draw_buttontop(&ts_wid_sel);
}


static void stuff_with_cur_ix(Button *m)
{
SHORT *v;

v = m->datme;
*v = vs.frame_ix;
redraw_tseg(m);
}

