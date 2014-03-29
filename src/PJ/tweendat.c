#include "jimk.h"
#include "broadcas.h"
#include "errcodes.h"
#include "grid.h"
#include "inks.h"
#include "mask.h"
#include "pentools.h"
#include "rastcurs.h"
#include "softmenu.h"
#include "tween.h"
#include "tweenpul.h"
#include "zoom.h"

static Button twe_grid_sel = MB_INIT1(
	NONEXT,
	NOCHILD, /* children */
	44, 9, 267, 14, /* w,h,x,y */
	NODATA,		/* Read in from menu file */
	ncorner_text,
	toggle_bgroup,
	qgrid_keep_undo,
	&vs.use_grid,1,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button twe_mask_sel = MB_INIT1(
	&twe_grid_sel, /* next */
	NOCHILD,
	44, 9, 219, 14, /* w,h,x,y */
	NODATA,		/* Read in from menu file */
	see_mask_button,
	mb_toggle_mask,
	qmask_keep_undo, 
	&vs.use_mask,1,
	NOKEY,
	MB_B_GHILITE
	);
static Button twe_zpan_sel = MB_INIT1(
	&twe_mask_sel,
	&zpan_cycle_group,
	39, 9, 13, 14, /* w,h,x,y */
	NODATA,
	hang_children,
	NOFEEL,
	NOOPT,
	NULL,0,
	NOKEY,
	0
	);
static Button twe_minipal_sel = MB_INIT1(
	&twe_zpan_sel,
	&minipal_sel,
	93, 9, 119, 14, /* w,h,x,y */
	NOTEXT,
	hang_children,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button twe_goinks_sel = MB_INIT1(
	&twe_minipal_sel,
	NOCHILD,
	53,9,238,3,
	NOTEXT,
	see_cur_ink,
	qinks,
	qinks,
	&vs.ink_id,opq_INKID,
	NOKEY,
	MB_GHILITE
	);
static Sgroup1_data twe_sh1dat = {
	&flxtime_data,
	};
static Button twe_std1_sel = MB_INIT1(
	&twe_goinks_sel,
	&std_head1_sel,
	0, 0, 129, 3, /* w,h,x,y */
	NOTEXT,
	hang_children,
	NOFEEL,
	NOOPT,
	&twe_sh1dat,0,
	NOKEY,
	0 /* flags */
	);
static Button twe_tool_sel = MB_INIT1(
	&twe_std1_sel,
	NOCHILD,
	64, 9, 63, 3, /* w,h,x,y */
	NODATA,		/* Read in from menu file */
	dcorner_text,
	twe_go_tool,
	twe_go_tool,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);
static Button twe_title_sel = MB_INIT1(
	&twe_tool_sel,
	NOCHILD,
	54, 9, 5, 3, 	/* w,h,x,y */
	NODATA,		/* Read in from menu file */
	see_titlebar,
	feel_titlebar,
	mb_menu_to_bottom,
	&tbg_moveclose,0,
	'q',
	0 /* flags */
	);

static void twemenu_credraw(void *dat, USHORT why)
{
	(void)dat;
	(void)why;

	redraw_head1_ccolor(&twe_std1_sel);
	zpan_ccycle_redraw(&twe_zpan_sel);
	draw_button(&twe_minipal_sel);
}
static Redraw_node twemenu_rn = {
	{ NULL, NULL }, /* node */
	twemenu_credraw,
	NULL,
	NEW_CCOLOR };

static void twemenu_on_showhide(Menuhdr *mh,Boolean showing)
/* also used in paste menu which uses same common buttons */
{
	(void)mh;

	if(showing)
		add_color_redraw(&twemenu_rn);
	else
		rem_color_redraw(&twemenu_rn);
}
Menuhdr twe_menu = MENU_INIT0(
	320,25,0,0,   /* width, height, x, y */
	TWEEN_MUID,		/* id */
	PANELMENU,		/* type */
	&twe_title_sel,	/* buttons */
	SCREEN_FONT,	/* font */
	&menu_cursor.hdr,	/* cursor */
	seebg_white, 		/* seebg */
	NULL,					/* dodata */
	NULL,					/* domenu */
	(MBPEN|MBRIGHT|KEYHIT), /* ioflags */
	0,				/* flags */
	NULL,			/* procmouse */
	twemenu_on_showhide, /* on_showhide */
	NULL			/* cleanup */
);

static Smu_button_list twe_smblist[] = {
	{ "title",  { /* butn */ &twe_title_sel } },
	{ "grid",   { /* butn */ &twe_grid_sel } },
	{ "mask",   { /* butn */ &twe_mask_sel } },
	{ "tool",   { /* butn */ &twe_tool_sel } },

	/* texts with first char a 'T' */
	{ "Ttoolname",  { /* ps */ &tween_pen_tool.ot.name } },
};


Errcode load_tween_panel_strings(void **ss)
/* Load up strings associated with tween panel */
{
	return(soft_buttons("tween_panel",twe_smblist,Array_els(twe_smblist),ss));
}
