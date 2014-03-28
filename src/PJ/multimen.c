/* multimen.c - Data structures and code associated with the "Time Select"
   menu that gets called by almost anything that does something over
   move than one frame.  Basic argument to this module is an "auto-vec"
   which will produce a transformed vb.pencel given the original
   render form and an up-to-date undof->  (undo buffer).  This guy lets
   you select the part of your FLIC you want to transform, specify
   ease in/ease out, go visit the palette editor, do a pixel perfect
   (but slow) preview, and finally actually call do_auto with the 
   auto-vec. */

#include "jimk.h"
#include "auto.h"
#include "brush.h"
#include "celmenu.h"
#include "errcodes.h"
#include "inks.h"
#include "mask.h"
#include "menus.h"
#include "palmenu.h"
#include "pentools.h"
#include "rastcurs.h"
#include "softmenu.h"

static Autoarg *mum_autoarg;

extern void
	change_time_mode();

extern Button tseg_group_sel;
extern Menuhdr quick_menu;

static void multi_use(Button *b);
static void multi_preview(void);
static void multi_go_inks(void);

/*** Button Data ***/

static Button mum_toa_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	75,11,241,55,
	NODATA,
	ccorner_text,
	change_time_mode,
	NOOPT,
	&vs.time_mode,2,
	'a',
	MB_GHILITE
	);

static Button mum_tos_sel = MB_INIT1(
	&mum_toa_sel,
	NOCHILD,
	75,11,241,(172)-130,
	NODATA,
	ccorner_text,
	change_time_mode,
	NOOPT,
	&vs.time_mode,1,
	's',
	MB_GHILITE
	);
static Button mum_tof_sel = MB_INIT1(
	&mum_tos_sel,
	NOCHILD,
	75,11,241,29,
	NODATA,
	ccorner_text,
	change_time_mode,
	NOOPT,
	&vs.time_mode,0,
	'f',
	MB_GHILITE
	);

static Button mum_cel_sel = MB_INIT1(
	&mum_tof_sel,
	NOCHILD,
	52,11,187,29,
	NODATA, /* "Next Cel", */
	ccorner_text,
	toggle_bgroup,
	go_nodraw_cel_menu,
	&vs.paste_inc_cel,1,
	NOKEY,
	MB_GHILITE
	);

static Button mum_com_sel = MB_INIT1(
	&mum_cel_sel,
	NOCHILD,
	54,11,131,(185)-130,
	NODATA,
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.ado_complete,1,
	NOKEY,
	MB_B_GHILITE
	);
static Button mum_out_sel = MB_INIT1(
	&mum_com_sel,
	NOCHILD,
	54,11,131,(172)-130,
	NODATA,
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.ado_ease_out,1,
	NOKEY,
	MB_B_GHILITE
	);
static Button mum_ins_sel = MB_INIT1(
	&mum_out_sel,
	NOCHILD,
	54,11,131,(159)-130,
	NODATA,
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.ado_ease,1,
	NOKEY,
	MB_B_GHILITE
	);
static Button mum_rev_sel = MB_INIT1(
	&mum_ins_sel,
	NOCHILD,
	60,11,69,(185)-130,
	NODATA,
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.ado_reverse,1,
	NOKEY,
	MB_B_GHILITE
	);
static Button mum_pin_sel = MB_INIT1(
	&mum_rev_sel,
	NOCHILD,
	60,11,69,(172)-130,
	NODATA,
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.ado_pong,1,
	NOKEY,
	MB_B_GHILITE
	);
static Button mum_sti_sel = MB_INIT1(
	&mum_pin_sel,
	NOCHILD,
	60,11,69,(159)-130,
	NODATA,
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.ado_tween,0,
	NOKEY,
	MB_B_GHILITE
	);
static Button mum_ok_sel = MB_INIT1(
	&mum_sti_sel,
	NOCHILD,
	49,11,9,(185)-130,
	NODATA, /* "Render", */
	dcorner_text,
	multi_use,
	NOOPT,
	NOGROUP,0,
	'r',
	0
	);
static Button mum_pre_sel = MB_INIT1(
	&mum_ok_sel,
	NOCHILD,
	49,11,9,(172)-130,
	NODATA, /* "Preview" */
	dcorner_text,
	multi_preview,
	NOOPT,
	NOGROUP,0,
	'p',
	0
	);
static Button mum_can_sel = MB_INIT1(
	&mum_pre_sel,
	NOCHILD,
	49,11,9,29,
	NODATA, /* "Cancel" */
	dcorner_text,
	mb_close_cancel,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button mum_tse_sel = MB_INIT1(
	&mum_can_sel,
	&tseg_group_sel,
	311+1,11,4,15,
	NOTEXT,
	hang_children,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button mum_cco_sel = MB_INIT1(
	&mum_tse_sel,
	NOCHILD,
	11,9,305,3,
	NOTEXT,
	ccolor_box,
	go_color_grid,
	ppalette,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button mum_bru_sel = MB_INIT1(
	&mum_cco_sel,
	NOCHILD,
	11,11,288,2,
	NOTEXT,
	see_pen,
	toggle_pen,
	set_pbrush,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button mum_kmo_sel = MB_INIT1(
	&mum_bru_sel,
	NOCHILD,
	11,9,272,3,
	NODATA,
	ncorner_text,
	toggle_bgroup,
	go_cel_menu,
	&vs.zero_clear,1,
	NOKEY,
	MB_B_GHILITE
	);
static Button mum_smo_sel = MB_INIT1(
	&mum_kmo_sel,
	NOCHILD,
	11,9,257,3,
	NODATA,
	ncorner_text,
	toggle_bgroup,
	qmask,
	&vs.use_mask,1,
	NOKEY,
	MB_B_GHILITE
	);
static Button mum_fmo_sel = MB_INIT1(
	&mum_smo_sel,
	NOCHILD,
	11,9,242,3,
	NODATA,
	ncorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.fillp,1,
	NOKEY,
	MB_B_GHILITE
	);
static Button mum_mmo_sel = MB_INIT1(
	&mum_fmo_sel,
	NOCHILD,
	11,9,227,3,
	NODATA,
	ncorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.multi,1,
	NOKEY,
	MB_B_GHILITE
	);
static Button mum_ink_sel = MB_INIT1(
	&mum_mmo_sel,
	NOCHILD,
	39,9,184,3,
	NOTEXT,
	see_cur_ink,
	multi_go_inks,
	multi_go_inks,
	&vs.ink_id,opq_INKID,
	NOKEY,
	MB_GHILITE
	);

extern Minitime_data flxtime_data;

static Button mum_min_sel = MB_INIT1(
	&mum_ink_sel,
	&minitime_sel,
	77+1,8+1,101,3,
	NOTEXT,
	hang_children,
	NOFEEL,
	NOOPT,
	(SHORT *)&flxtime_data,0,
	NOKEY,
	0
	);
static Button mum_tit_sel = MB_INIT1(
	&mum_min_sel,
	NOCHILD,
	92,9,4,3,
	NODATA,
	see_titlebar,
	feel_titlebar,
	mb_menu_to_bottom,
	&tbg_moveclose,0,
	'q',
	0
	);

static void mum_seebg_white(Menuwndo *m)
{
set_button_disable(&mum_smo_sel,(mask_rast == NULL));
seebg_white(m);
}

Menuhdr mum_menu = {
	{320,70,0,130}, /* width, height, x, y */
	MULTI_MUID,		/* id */
	PANELMENU,		/* type */
	&mum_tit_sel,	/* buttons */
	SCREEN_FONT,	/* font */
	&menu_cursor.hdr,	/* cursor */
	mum_seebg_white, 	/* seebg */
	NULL,				/* dodata */
	NULL,				/* domenu */
	(MBPEN|MBRIGHT|KEYHIT), /* ioflags */
	0,				/* flags */
	NULL,			/* mw */
	NULL,			/* group */
	{ NULL, NULL },	/* node */
	NULL,			/* procmouse */
	NULL,			/* on_showhide */
	NULL,			/* cleanup */
	0, 0, 0, 0		/* scaled width, height, x, y */
};

/* two sets of keys first chars are different! */

static char xframe_key[] = "Xframe";
static char xseg_key[] = "Xseg";
static char xall_key[] = "Xall";

static void set_tofrom(int from)
{
	xframe_key[0] = xseg_key[0] = xall_key[0] = (from?'f':'t');
}

static Smu_button_list mum_smblist[] = {
	{ "title",      { &mum_tit_sel } },
	{ "nextcel",    { &mum_cel_sel } },
	{ "complete",   { &mum_com_sel } },
	{ "out_slow",   { &mum_out_sel } },
	{ "in_slow",    { &mum_ins_sel } },
	{ "reverse",    { &mum_rev_sel } },
	{ "ppong",      { &mum_pin_sel } },
	{ "still",      { &mum_sti_sel } },
	{ "render",     { &mum_ok_sel } },
	{ "preview",    { &mum_pre_sel } },
	{ "cancel",     { &mum_can_sel } },
	{ "key",        { &mum_kmo_sel } },
	{ "msk",        { &mum_smo_sel } },
	{ "fill",       { &mum_fmo_sel } },
	{ "otime",      { &mum_mmo_sel } },
	{ "pen",        { &mum_bru_sel } },
	{ xall_key,     { &mum_toa_sel } },
	{ xseg_key,     { &mum_tos_sel } },
	{ xframe_key,   { &mum_tof_sel } },
};

static void multi_use(Button *b)
{
Errcode err;

	if(!mum_autoarg)
	{
		err = Err_abort;
		goto closeup;
	}

	if(mum_autoarg->flags & AUTO_PREVIEW_ONLY)
	{
		err = Success;
		goto closeup;
	}

	if(mum_autoarg->avec != NULL)
	{
		hide_mp();
		if((err = noask_do_auto_time_mode(mum_autoarg)) == Err_abort)
		{
			show_mp(); /* if aborted, bring multi menu back */
			return;
		}
		/* note: we dont need a show_mp() if closing menu group */
	}
	else
		err = Err_abort;

closeup:
	mb_gclose_code(b,err);
}

static void multi_preview(void)
{
Errcode err;

	if(mum_autoarg && mum_autoarg->avec != NULL)
	{
		hide_mp();
		mum_autoarg->in_preview = TRUE;
		err = dopreview(mum_autoarg);
		mum_autoarg->in_preview = FALSE;
		softerr(err,"cel_preview");
		show_mp();
	}
}

static void multi_go_inks(void)
/* Disable redo button before going to inks. */
{
VFUNC oredo;

oredo = vl.redoit;
vl.redoit = NULL;
qinks();
vl.redoit = oredo;
}

Errcode multimenu(Autoarg *aa)
{
Errcode err;
VFUNC oundo;
void *ss;
static Button *disabtab[] = {
	&mum_pre_sel,
	&mum_ok_sel,
	NULL,
};

	if(MENU_ISOPEN(&mum_menu)) /* avoid recursion */
		return(Err_abort);
	set_mbtab_disables(disabtab,(aa == NULL || aa->avec == NULL));

	hide_mp();

	set_tofrom(	aa->flags & AUTO_READONLY );

	if((err = soft_buttons("multi_panel", mum_smblist, 
					 Array_els(mum_smblist), &ss)) < Success)
	{
		goto error;
	}

	oundo = vl.undoit;
	vl.undoit = NULL;	/* undo/redo not possible from here */
	mum_autoarg = aa;
	clip_tseg();
	menu_to_cursor(vb.screen, &mum_menu);
	err = do_reqloop(vb.screen,&mum_menu,NULL,NULL,NULL);
	vl.undoit = oundo;
	add_check_tflx_toram(); /* something here may have forced it out */

error:
	smu_free_scatters(&ss);
	show_mp();
	return(err);
}

void disable_multi_menu()
{
	mum_menu.flags |= MENU_DISABLED;
}
void enable_multi_menu()
{
	mum_menu.flags &= ~MENU_DISABLED;
}
void go_multi(void)
{
Autoarg aa;
extern Menuhdr mum_menu;

	if(mum_menu.flags & MENU_DISABLED)
		return;
	clear_struct(&aa);
	multimenu(&aa);
}
#ifdef SLUFFED
void go_multi_read(void)
{
Autoarg aa;
extern Menuhdr mum_menu;

	if(mum_menu.flags & MENU_DISABLED)
		return;
	clear_struct(&aa);
	aa.flags = AUTO_READONLY;
	multimenu(&aa);
}
#endif /* SLUFFED */

