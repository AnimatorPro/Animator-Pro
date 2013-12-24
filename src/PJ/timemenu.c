
/* undone */
/* timemenu.c - The data structures and some associated code for the
   Frames control panel. and some other frames control button stuff */

#include "jimk.h"
#include "menus.h"
#include "fli.h"
#include "input.h"
#include "softmenu.h"

extern void ccolor_box(), ppalette(), see_pen(), toggle_pen(), set_pbrush(),
	insert_a_frame(), kill_a_frame(), set_total_frames(),
	minsert(), mdelete_frames(), jump_to_frame(), jump_to_mark(), set_mark(),
	spread_frames(), go_multi(), 
	see_range_button(), use_range_button(), set_range_button(),
	multi_preview(), multi_use();

#define TR1 155
#define TR2 187

extern Button tseg_group_sel;

extern Menuhdr quick_menu;

static SHORT jiffies;
Qslider speed_sl = QSL_INIT1(0, 120, &jiffies, 0, NULL, leftright_arrs);

static void see_speed_sl(Button *b)
{
	jiffies = millisec_to_jiffies(flix.hdr.speed);
	see_qslider(b);
}
static void feel_speed_sl(Button *b)
{
SHORT ojiffies;

	ojiffies = jiffies = millisec_to_jiffies(flix.hdr.speed);
	feel_qslider(b);
	if(jiffies != ojiffies)
		flix.hdr.speed = jiffies_to_millisec(jiffies);
}

void redraw_range_buttons();


static Button tmu_t5_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	17,11,253,(155)-137,
	NODATA, /* "*5", */
	ccorner_text,
	spread_frames,
	NOOPT,
	NOGROUP,5,
	NOKEY,
	0
	);
static Button tmu_t3_sel = MB_INIT1(
	&tmu_t5_sel,
	NOCHILD,
	17,11,234,(155)-137,
	NODATA, /* "*3", */
	ccorner_text,
	spread_frames,
	NOOPT,
	NOGROUP,3,
	NOKEY,
	0
	);
static Button tmu_t2_sel = MB_INIT1(
	&tmu_t3_sel,
	NOCHILD,
	17,11,215,(155)-137,
	NODATA, /* "*2", */
	ccorner_text,
	spread_frames,
	NOOPT,
	NOGROUP,2,
	NOKEY,
	0
	);
static Button tmu_md_sel = MB_INIT1(
	&tmu_t2_sel,
	NOCHILD,
	13,11,194+1,(155)-137,
	NODATA, /* "D", */
	dcorner_text,
	jump_to_mark,
	set_mark,
	&vs.frame_ix,1000-1,
	NOKEY,
	MB_GHILITE
	);
static Button tmu_mc_sel = MB_INIT1(
	&tmu_md_sel,
	NOCHILD,
	13,11,179+1,(155)-137,
	NODATA, /* "C", */
	dcorner_text,
	jump_to_mark,
	set_mark,
	&vs.frame_ix,100-1,
	NOKEY,
	MB_GHILITE
	);
static Button tmu_mb_sel = MB_INIT1(
	&tmu_mc_sel,
	NOCHILD,
	13,11,164+1,(155)-137,
	NODATA, /* "B", */
	dcorner_text,
	jump_to_mark,
	set_mark,
	&vs.frame_ix,10-1,
	NOKEY,
	MB_GHILITE
	);
static Button tmu_ma_sel = MB_INIT1(
	&tmu_mb_sel,
	NOCHILD,
	13,11,149+1,(155)-137,
	NODATA, /* "A", */
	dcorner_text,
	jump_to_mark,
	set_mark,
	&vs.frame_ix,0,
	NOKEY,
	MB_GHILITE	
	);
static Button tmu_marks_sel = MB_INIT1(
	&tmu_ma_sel,
	NOCHILD,
	33,8+1,111+3,(155)-137,
	NODATA, /* "MARKS", */
	black_ctext,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button tmu_rd_sel = MB_INIT1(
	&tmu_marks_sel,
	NOCHILD,
	13,11,92+4,(155)-137,
	NODATA, /* "D", */
	see_range_button,
	use_range_button,
	set_range_button,
	NOGROUP,3,
	NOKEY,
	0
	);
static Button tmu_rc_sel = MB_INIT1(
	&tmu_rd_sel,
	NOCHILD,
	13,11,77+4,(155)-137,
	NODATA, /* "C", */
	see_range_button,
	use_range_button,
	set_range_button,
	NOGROUP,2,
	NOKEY,
	0
	);
static Button tmu_rb_sel = MB_INIT1(
	&tmu_rc_sel,
	NOCHILD,
	13,11,62+4,(155)-137,
	NODATA, /* "B", */
	see_range_button,
	use_range_button,
	set_range_button,
	NOGROUP,1,
	NOKEY,
	0
	);
static Button tmu_ra_sel = MB_INIT1(
	&tmu_rb_sel,
	NOCHILD,
	13,11,47+4,(155)-137,
	NODATA, /* "A", */
	see_range_button,
	use_range_button,
	set_range_button,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button tmu_ranges_sel = MB_INIT1(
	&tmu_ra_sel,
	NOCHILD,
	39,8+1,6,(155)-137,
	NODATA, /* "Segment", */
	black_ctext,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button tmu_tseg_hanger = MB_INIT1(
	&tmu_ranges_sel,
	&tseg_group_sel,
	312,11,4,(170)-137,
	NOTEXT,
	hang_children,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button tmu_playr_sel = MB_INIT1(
	&tmu_tseg_hanger,
	NOCHILD,
	82,11,228,(185)-137,
	NODATA, /* "Time Select", */
	ncorner_text,
	toggle_bgroup,
	go_multi,
	&vs.multi,1,
	NOKEY,
	MB_B_GHILITE,	
	);
static Button tmu_spdsl_sel = MB_INIT1(
	&tmu_playr_sel,
	NOCHILD,
	148,11,70,(185)-137,
	&speed_sl,
	see_speed_sl,
	feel_speed_sl,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button tmu_spdtag_sel = MB_INIT1(
	&tmu_spdsl_sel,
	NOCHILD,
	65,8+1,3,(186)-137,
	NODATA, /* "Play speed", */
	black_ctext,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button tmu_frame_ct_sel = MB_INIT1(
	&tmu_spdtag_sel,
	NOCHILD,
	28,11,242,(141)-137,
	&flix.hdr.frame_count,
	ncorner_short,
	set_total_frames,
	set_total_frames,
	NOGROUP,0,
	NOKEY,
	MB_DISABOPT,
	);

static Button tmu_tslider_sel = MB_INIT1(
	&tmu_frame_ct_sel,
	&timeslider_sel,
	13,11,49,4,
	NODATA,
	hang_children,
	NOFEEL,
	NOOPT,
	&flxtime_data,0,
	NOKEY,
	0
	);
static Button tmu_kill_sel = MB_INIT1(
	&tmu_tslider_sel,
	NOCHILD,
	42,11,274,(155)-137,
	NODATA, /* "Delete", */
	ccorner_text,
	kill_a_frame,
	mdelete_frames,
	NOGROUP,0,
	NOKEY,
	MB_DISABOPT,
	);
static Button tmu_insert_sel = MB_INIT1(
	&tmu_kill_sel,
	NOCHILD,
	42,11,274,(141)-137,
	NODATA, /* "Insert", */
	ccorner_text,
	insert_a_frame,
	minsert,
	NOGROUP,0,
	NOKEY,
	MB_DISABOPT,
	);
static Button tmu_moveq_sel = MB_INIT1(
	&tmu_insert_sel,
	NOCHILD,
	43,11,4,3,
	NODATA, /* "Frames", */
	see_titlebar,
	mb_clipmove_menu,
	mb_menu_to_bottom,
	NOGROUP,0,
	NOKEY,
	0
	);

Menuhdr tmu_menu = {
	{320,63,0,137},		/* width, height, x, y */
	TITLE_MUID,   		/* id */
	PANELMENU,			/* type */
	&tmu_moveq_sel, 	/* buttons */
	SCREEN_FONT, 		/* font */
	&menu_cursor,		/* cursor */
	seebg_white, 		/* seebg */
	NULL,				/* dodata */
	NULL,				/* domenu */
	(MBPEN|MBRIGHT|KEYHIT), /* ioflags */
};

static Smu_button_list tmu_smblist[] = {
	{ "title", &tmu_moveq_sel },
	{ "mul5", &tmu_t5_sel },
	{ "mul3", &tmu_t3_sel },
	{ "mul2", &tmu_t2_sel },
	{ "jmp_d", &tmu_md_sel },
	{ "jmp_c", &tmu_mc_sel },
	{ "jmp_b", &tmu_mb_sel },
	{ "jmp_a", &tmu_ma_sel },
	{ "marks", &tmu_marks_sel },
	{ "use_d", &tmu_rd_sel },
	{ "use_c", &tmu_rc_sel },
	{ "use_b", &tmu_rb_sel },
	{ "use_a", &tmu_ra_sel },
	{ "seg", &tmu_ranges_sel },
	{ "otime", &tmu_playr_sel },
	{ "speed", &tmu_spdtag_sel },
	{ "del", &tmu_kill_sel },
	{ "insert", &tmu_insert_sel },
};

static void set_mark(Button *m)
{
int ix;

	ix = slist_ix(&tmu_ma_sel, m);
	m->identity = vs.marks[ix] = vs.frame_ix;
	change_mode(m);
}
static void jump_to_mark(Button *b)
{
SHORT t = b->identity;

	if (t >= flix.hdr.frame_count)
		t = flix.hdr.frame_count-1;
	mini_seek_frame(&flxtime_data,t);
	t = flix.hdr.frame_count;
	mb_draw_ghi_group(b);
	update_time_sel(&tmu_tslider_sel);
}
static void insert_some(int frames)
{
	hide_mp();
	unzoom();
	flx_clear_olays();
	scrub_frame_save_undo();
	insert_frames(frames, vs.frame_ix);
	flx_draw_olays();
	rezoom();
	show_mp();
}

void insert_a_frame(void)
{
	insert_some(1);
}

static void minsert(Button *b)
{
	hide_mp();
	unzoom();
	qinsert_frames();
	rezoom();
	show_mp();
}

static void mdelete_frames(Button *b)
{
	hide_mp();
	unzoom();
	qdelete_frames();
	rezoom();
	show_mp();
}

void kill_a_frame(void)
{
	hide_mp();
	unzoom();
	if (soft_yes_no_box("frame_del"))
		delete_some(1);
	rezoom();
	show_mp();
}

void set_total_frames()
{
	hide_mp();
	unzoom();
	qmake_frames();
	rezoom();
	show_mp();
}

static void set_range_button(Button *m)
{
int ix;

	ix = m->identity;
	vs.starttr[ix] = vs.start_seg;
	vs.stoptr[ix] = vs.stop_seg;
	redraw_range_buttons();
}

static void see_range_button(Button *b)
{
SHORT ix;

	ix = b->identity;
	mb_set_hilite(b, (vs.start_seg == vs.starttr[ix] 
				  	   && vs.stop_seg == vs.stoptr[ix]));
	dcorner_text(b);
}

void redraw_range_buttons(void)
{
	draw_buttontop(&tmu_ra_sel);
	draw_buttontop(&tmu_rb_sel);
	draw_buttontop(&tmu_rc_sel);
	draw_buttontop(&tmu_rd_sel);
}

static void use_range_button(Button *m)
{
int ix;

	ix = m->identity;
	vs.start_seg = vs.starttr[ix];
	vs.stop_seg = vs.stoptr[ix];
	clip_tseg();
	redraw_tseg(m);
	redraw_range_buttons();
}


static void attatch_marks(void)
{
SHORT *mark;
Button *butn;
int i;

	i = 4;
	butn= &tmu_ma_sel;
	mark = vs.marks;
	while (--i >= 0)
	{
		butn->identity = *mark++;
		butn= butn->next;
	}
}

static void spread_frames(Button *m)
{
Flx *fdest;
Flx *fsrc;
int scount;
int dcount;
int mult;

	hide_mp();
	unzoom();
	flx_clear_olays();
	scrub_cur_frame();

	mult = m->identity;
	scount = flix.hdr.frame_count;
	dcount = mult * scount;

	/* Multiply frames.  First expand flx to have the number of frames
	 * for the result insert new empty frames at end of flx.  Note this will
	 * move the ring frame to the end of the flx and separate it from the last
	 * frame by a bunch of blanks */

	if(insert_frames(dcount - scount,scount - 1) < Success)
		goto error;

	/* Now we must space out the index entries separating each by mult-1
	 * empty records, records with a size of 0 */

	fdest = &flix.idx[dcount]; /* will be set to address of ring frame */
	fsrc = &flix.idx[scount];  /* set to address of old ring frame */

	mult -= 1;
	while(fdest > flix.idx)
	{
		fdest -= mult; /* skip mult -1 back and clear it (make empty) */
		clear_mem(fdest,sizeof(Flx)*mult);
		--fdest;  /* go back one more */
		--fsrc;
		*fdest = *fsrc;
	}


#ifdef SLUFFED

int i;

	/* this is a more generic way but the above is faster and more stable
	 * for the current index structure */

	for (i=0; i<scount; i++)
	{
		if((insert_frames(mult-1, i*mult)) < Success);
		{
			vs.bframe_ix = 0;
			goto done;
		}
	}
#endif /* SLUFFED */

	mult += 1;
	vs.frame_ix *= mult;
	vs.bframe_ix *= mult;
error:
	flx_draw_olays();
	rezoom();
	show_mp();
}

Minitime_data *omtd;
static void tmu_draw_olays(void *dat)
{
	mb_draw_ghi_group(&tmu_ma_sel);
	if(omtd->draw_overlays && (omtd->olay_stack <= 0))
		omtd->draw_overlays(dat);
}
static void tmu_clear_olays(void *dat)
{
	if(omtd->clear_overlays && (omtd->olay_stack <= 0))
		omtd->clear_overlays(dat);
}

static Button *ovl_disab_butns[] = {
	&tmu_kill_sel,
	&tmu_insert_sel,
	&tmu_t5_sel,
	&tmu_t3_sel,
	&tmu_t2_sel,
	&tmu_frame_ct_sel,
	NULL,
};
static void tmu_disables()
{
	set_mbtab_disables(ovl_disab_butns,(flix.overlays != NULL));
}

void do_time_menu(void)
{
void *ss;
Minitime_data omt;

	if(MENU_ISOPEN(&tmu_menu)) /* no recursion */
		return;
	if((soft_buttons("frames_panel", tmu_smblist, 
					Array_els(tmu_smblist), &ss)) < Success)
	{
		return;
	}
	menu_to_quickcent(&tmu_menu);

	omt = flxtime_data;
	omtd = &omt;
	flxtime_data.draw_overlays = tmu_draw_olays;
	flxtime_data.clear_overlays = tmu_clear_olays;
	flxtime_data.olay_stack = 0;

	clip_tseg();
	attatch_marks();
	tmu_disables();
	do_reqloop(vb.screen,&tmu_menu,NULL,NULL,NULL);
	smu_free_scatters(&ss);
	flxtime_data = omt;
}

#ifdef SLUFFED
void disable_time_menu()
{
	tmu_menu.flags |= MENU_DISABLED;
}
void enable_time_menu()
{
	tmu_menu.flags &= ~MENU_DISABLED;
}
#endif /* SLUFFED */
void go_time_menu(void)
{
	if(tmu_menu.flags & MENU_DISABLED)
		return;
	if(MENU_ISOPEN(&tmu_menu)) /* no recursion */
		return;
	hide_mp();
	do_time_menu();
	show_mp();
}
