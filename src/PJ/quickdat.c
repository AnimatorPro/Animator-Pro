/* quickdat.c - the data structures for the main control panel.  6 tools,
   6 inks, etc.   Also some code at end for some of the buttons feelme
   and seeme's.
   */

#include "jimk.h"
#include "broadcas.h"
#include "filemenu.h"
#include "grid.h"
#include "homepul.h"
#include "inks.h"
#include "input.h"
#include "mask.h"
#include "menus.h"
#include "palmenu.h"
#include "pentools.h"
#include "rastcurs.h"
#include "softmenu.h"
#include "zoom.h"

extern void
	toggle_pen(),
	set_pspeed(), toggle_draw_mode(),
	movefli_tool(), go_multi(), toggle_stencil();

static Sgroup1_data qmu_sh1dat = {
	&flxtime_data,
};

Button qmu_clus_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	78, 9, 128, 44, /* w,h,x,y */
	NOTEXT,
	see_crb,
	feel_crb,
	ppalette,
	NULL,0,
	NOKEY,
	0
	);

void see_undo(Button *b)
{
set_button_disable(b,(vl.undoit == NULL));
ccorner_text(b);
}
void see_redo(Button *b)
{
set_button_disable(b,(vl.redoit == NULL));
ccorner_text(b);
}

void see_clusid(Button *b)
{
char clusid[2];
char *dat;

	dat = b->datme;
	clusid[0] = dat[vs.use_bun]; /* 1 or 0 */
	clusid[1] = 0;
	b->datme = clusid;
	ccorner_text(b);
	b->datme = dat;
}
void toggle_clusid(Button *b)
{
	set_use_bun(!vs.use_bun);
	draw_buttontop(b);
	draw_buttontop(b->group);
}
static Button qmu_clusid_sel = MB_INIT1(
	&qmu_clus_sel,
	NOCHILD,
	12, 9, 114, 44, /* w,h,x,y */
	NODATA,
	see_clusid,
	toggle_clusid,
	ppalette,
	&qmu_clus_sel,0,
	NOKEY,
	0
	);

static Button qmu_minipal_sel = MB_INIT1(
	&qmu_clusid_sel,
	&minipal_sel,
	29, 9, 114, 24, /* w,h,x,y */
	NOTEXT,
	hang_children,
	NOFEEL,
	NOOPT,
	NULL,0,
	NOKEY,
	0
	);

/****** pen tool group ******/

static Button ptg_opt7_sel = MB_INIT1(
	NONEXT, /* next */
	NOCHILD, /* children */
	53, 9, 55, 30, /* w,h,x,y */
	NOTEXT, /* datme */
	see_option_name,
	change_pen_mode,
	go_dtoolopts,
	&vs.ptool_id,0,
	NOKEY,
	MB_GHILITE /* flags */
	);
static Button ptg_opt6_sel = MB_INIT1(
	&ptg_opt7_sel, /* next */
	NOCHILD, /* children */
	53, 9, 0, 30, /* w,h,x,y */
	NOTEXT, /* datme */
	see_option_name,
	change_pen_mode,
	go_dtoolopts,
	&vs.ptool_id,0,
	NOKEY,
	MB_GHILITE /* flags */
	);
static Button ptg_opt5_sel = MB_INIT1(
	&ptg_opt6_sel, /* next */
	NOCHILD, /* children */
	53, 9, 55, 20, /* w,h,x,y */
	NOTEXT, /* datme */
	see_option_name,
	change_pen_mode,
	go_dtoolopts,
	&vs.ptool_id,0,
	NOKEY,
	MB_GHILITE /* flags */
	);
static Button ptg_opt4_sel = MB_INIT1(
	&ptg_opt5_sel, /* next */
	NOCHILD, /* children */
	53, 9, 0, 20, /* w,h,x,y */
	NOTEXT, /* datme */
	see_option_name,
	change_pen_mode,
	go_dtoolopts,
	&vs.ptool_id,0,
	NOKEY,
	MB_GHILITE /* flags */
	);
static Button ptg_opt3_sel = MB_INIT1(
	&ptg_opt4_sel, /* next */
	NOCHILD, /* children */
	53, 9, 55, 10, /* w,h,x,y */
	NOTEXT, /* datme */
	see_option_name,
	change_pen_mode,
	go_dtoolopts,
	&vs.ptool_id,0,
	NOKEY,
	MB_GHILITE /* flags */
	);
static Button ptg_opt2_sel = MB_INIT1(
	&ptg_opt3_sel, /* next */
	NOCHILD, /* children */
	53, 9, 0, 10, /* w,h,x,y */
	NOTEXT, /* datme */
	see_option_name,
	change_pen_mode,
	go_dtoolopts,
	&vs.ptool_id,0,
	NOKEY,
	MB_GHILITE /* flags */
	);
static Button ptg_opt1_sel = MB_INIT1(
	&ptg_opt2_sel, /* next */
	NOCHILD, /* children */
	53, 9, 55, 0, /* w,h,x,y */
	NOTEXT, /* datme */
	see_option_name,
	change_pen_mode,
	go_dtoolopts,
	&vs.ptool_id,0,
	NOKEY,
	MB_GHILITE /* flags */
	);
Button pen_opts_sel = MB_INIT1(
	&ptg_opt1_sel, /* next */
	NOCHILD, /* children */
	53, 9, 0, 0, /* w,h,x,y */
	NOTEXT, /* datme */
	see_option_name,
	change_pen_mode,
	go_dtoolopts,
	&vs.ptool_id,0,
	NOKEY,
	MB_GHILITE /* flags */
	);

/****** ink group ******/

static Button ig_ink7_sel = MB_INIT1(
	NONEXT, /* next */
	NOCHILD, /* children */
	53, 9, 55, 30, /* w,h,x,y */
	NOTEXT, /* datme */
	see_option_name,
	change_ink_mode,
	go_inkopts,
	&vs.ink_id,0,
	NOKEY,
	MB_GHILITE /* flags */
	);
static Button ig_ink6_sel = MB_INIT1(
	&ig_ink7_sel, /* next */
	NOCHILD, /* children */
	53, 9, 0, 30, /* w,h,x,y */
	NOTEXT, /* datme */
	see_option_name,
	change_ink_mode,
	go_inkopts,
	&vs.ink_id,0,
	NOKEY,
	MB_GHILITE /* flags */
	);
static Button ig_ink5_sel = MB_INIT1(
	&ig_ink6_sel, /* next */
	NOCHILD, /* children */
	53, 9, 55, 20, /* w,h,x,y */
	NOTEXT, /* datme */
	see_option_name,
	change_ink_mode,
	go_inkopts,
	&vs.ink_id,0,
	NOKEY,
	MB_GHILITE /* flags */
	);
static Button ig_ink4_sel = MB_INIT1(
	&ig_ink5_sel, /* next */
	NOCHILD, /* children */
	53, 9, 0, 20, /* w,h,x,y */
	NOTEXT, /* datme */
	see_option_name,
	change_ink_mode,
	go_inkopts,
	&vs.ink_id,0,
	NOKEY,
	MB_GHILITE /* flags */
	);
static Button ig_ink3_sel = MB_INIT1(
	&ig_ink4_sel, /* next */
	NOCHILD, /* children */
	53, 9, 55, 10, /* w,h,x,y */
	NOTEXT, /* datme */
	see_option_name,
	change_ink_mode,
	go_inkopts,
	&vs.ink_id,0,
	NOKEY,
	MB_GHILITE /* flags */
	);
static Button ig_ink2_sel = MB_INIT1(
	&ig_ink3_sel, /* next */
	NOCHILD, /* children */
	53, 9, 0, 10, /* w,h,x,y */
	NOTEXT, /* datme */
	see_option_name,
	change_ink_mode,
	go_inkopts,
	&vs.ink_id,0,
	NOKEY,
	MB_GHILITE /* flags */
	);
static Button ig_ink1_sel = MB_INIT1(
	&ig_ink2_sel, /* next */
	NOCHILD, /* children */
	53, 9, 55, 0, /* w,h,x,y */
	NOTEXT, /* datme */
	see_option_name,
	change_ink_mode,
	go_inkopts,
	&vs.ink_id,0,
	NOKEY,
	MB_GHILITE /* flags */
	);
Button ink_opts_sel = MB_INIT1(
	&ig_ink1_sel, /* next */
	NOCHILD, /* children */
	53, 9, 0, 0, /* w,h,x,y */
	NOTEXT, /* datme */
	see_option_name,
	change_ink_mode,
	go_inkopts,
	&vs.ink_id,0,
	NOKEY,
	MB_GHILITE /* flags */
	);

static Button qmu_grid_sel = MB_INIT1(
	&qmu_minipal_sel,
	NOCHILD, /* children */
	44, 9, 162, 34, /* w,h,x,y */
	NODATA, /* datme */
	ncorner_text,
	toggle_bgroup,
	qgrid,
	&vs.use_grid,1,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
void see_mask_button(Button *b)
{
	set_button_disable(b,(mask_rast == NULL));
	ncorner_text(b);
}
static Button qmu_mask_sel = MB_INIT1(
	&qmu_grid_sel, /* next */
	NOCHILD,
	46, 9, 114, 34, /* w,h,x,y */
	NODATA,
	see_mask_button,
	mb_toggle_mask,
	qmask,
	&vs.use_mask,1,
	NOKEY,
	MB_B_GHILITE
	);
static Button qmu_penopts_sel = MB_INIT1(
	&qmu_mask_sel,
	&pen_opts_sel, /* next */
	108, 39, 3, 14, /* w,h,x,y */
	NOTEXT,
	hang_children,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);

static Button qmu_inkopts_sel = MB_INIT1(
	&qmu_penopts_sel,
	&ink_opts_sel, /* next */
	108, 39, 209, 14, /* w,h,x,y */
	NOTEXT,
	hang_children,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);

static Button qmu_zpan_sel = MB_INIT1(
	&qmu_inkopts_sel, /* next */
	&zpan_cycle_group,
	39, 9, 114, 14, /* w,h,x,y */
	NODATA,
	hang_children,
	NOFEEL,
	NOOPT,
	NULL,0,
	NOKEY,	
	0
	);

static Button qmu_files_sel = MB_INIT1(
	&qmu_zpan_sel,
	NOCHILD,
	53,9,238,3,
	NODATA,
	ccorner_text,
	mb_go_files,
	mb_go_files,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button qmu_std1_sel = MB_INIT1(
	&qmu_files_sel, /* next */
	&std_head1_sel,
	0, 0, 129, 3, /* w,h,x,y */
	NOTEXT,
	hang_children,
	NOFEEL,
	NOOPT,
	&qmu_sh1dat,0,
	NOKEY,
	0 /* flags */
	);
static Button qmu_redo_sel = MB_INIT1(
	&qmu_std1_sel, /* next */
	NOCHILD, /* children */
	33, 9, 94, 3, /* w,h,x,y */
	NODATA, /* datme */
	see_redo,
	menu_doredo,
	NOOPT,
	NOGROUP,0,
	'r',
	0 /* flags */
	);
static Button qmu_undo_sel = MB_INIT1(
	&qmu_redo_sel, /* next */
	NOCHILD, /* children */
	33, 9, 59, 3, /* w,h,x,y */
	NODATA, /* datme */
	see_undo,
	menu_doundo,
	NOOPT,
	NOGROUP,0,
	'\b',
	0 /* flags */
	);

static Button qmu_title_sel = MB_INIT1(
	&qmu_undo_sel,
	NOCHILD,
	53,9,3,3,
	NODATA,
	see_titlebar,
	mb_move_quickmenu,
	mb_quickmenu_to_bottom,
	NOGROUP,0,
	NOKEY,
	0
	);

static void qmu_color_redraw(void *dat, USHORT why)
{
	(void)dat;
	(void)why;

	redraw_head1_ccolor(&qmu_std1_sel);
	zpan_ccycle_redraw(&qmu_zpan_sel);
	draw_button(&qmu_clus_sel);
	draw_button(&qmu_minipal_sel);
}

static Redraw_node quick_rn = {
	{ NULL, NULL }, /* node */
	qmu_color_redraw,
	NULL,
	NEW_CCOLOR };

static void qmu_on_showhide(Menuhdr *mh,Boolean showing)
{
	(void)mh;

	if(showing)
	{
		add_color_redraw(&quick_rn);
	}
	else
		rem_color_redraw(&quick_rn);
}

Menuhdr quick_menu = MENU_INIT0(
	320,55,0,165,
	QUICK_MUID,		/* id */
	PANELMENU,		/* type */
	&qmu_title_sel,	/* buttons */
	SCREEN_FONT,	/* font */
	&menu_cursor.hdr, /* cursor */
	seebg_white, 	/* seebg */
	NULL,			/* data */
	NULL,			/* domenu */
	(MBPEN|MBRIGHT|KEYHIT), /* ioflags */
	0,				/* flags */
	NULL,			/* procmouse */
	qmu_on_showhide, /* on_showhide */
	NULL			/* cleanup */
);

static Smu_button_list home_smblist[] = {
	{ "title",  { &qmu_title_sel } },
	{ "clusid", { &qmu_clusid_sel } },
	{ "grid",   { &qmu_grid_sel } },
	{ "mask",   { &qmu_mask_sel } },
	{ "files",  { &qmu_files_sel } },
	{ "redo",   { &qmu_redo_sel } },
	{ "undo",   { &qmu_undo_sel } },
};

extern void main_selit();
extern Boolean do_mainpull(Menuhdr *mh);

Errcode go_quick_menu(void)
{
Errcode err;
Menuhdr tpull;
void *ss;

	if((err = soft_buttons("home_panel", home_smblist, 
					 Array_els(home_smblist), &ss)) < Success)
	{
		goto error;
	}

	if((err = load_home_keys()) < Success)
		goto error;

	if ((err = load_soft_pull(&tpull, 0, "home", MAINP_MUID,
							  main_selit, do_mainpull)) < Success)
	{
		goto error;
	}
	init_poco_pull(&tpull, POC_DOT_PUL, POC_PUL);
	scale_pull(&tpull, 0);		/* scale poco stuff too... */
	err = do_menuloop(vb.screen,&quick_menu,NULL,
				      &tpull,home_dokeys);
	smu_free_pull(&tpull);

error:
	smu_free_scatters(&ss);
	err = softerr(err,"home_menu");	
	return(err);
}
void see_crb(Button *m)
{
	m->identity = vs.use_bun;
	see_cluster(m);
}

void feel_crb(Button *m)
{
	m->identity = vs.use_bun;
	feel_cluster(m);
}

