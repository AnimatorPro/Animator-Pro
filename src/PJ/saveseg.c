#include "jimk.h"
#include "commonst.h"
#include "fli.h"
#include "rastcurs.h"
#include "softmenu.h"

extern Button tseg_slider_sel;

static Button ssm_save_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	80,11,137,30,
	NODATA, /* "Save Segment", */
	ccorner_text,
	mb_gclose_ok,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button ssm_cancel_sel = MB_INIT1(
	&ssm_save_sel,
	NOCHILD,
	64,11,227,30,
	NODATA, /* "Cancel", */
	ccorner_text,
	mb_gclose_cancel,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button ssm_tseg_sel =  MB_INIT1(
	&ssm_cancel_sel,
	&tseg_slider_sel, /* children */
	0,0,24,16,
	NODATA,
	hang_children,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button ssm_tslider_sel = MB_INIT1(
	&ssm_tseg_sel,
	&timeslider_sel,
	37,9,98,3,
	NODATA,
	hang_children,
	NOFEEL,
	NOOPT,
	&flxtime_data,0,
	NOKEY,
	0
	);
static Button ssm_title_sel = MB_INIT1(
	&ssm_tslider_sel,
	NOCHILD,
	92,9,4,4,
	NODATA, /* "Segment Save", */
	see_titlebar,
	feel_titlebar,
	NOOPT,
	&tbg_moveclose,0,
	NOKEY,
	0
	);

static Menuhdr saveseg_menu = MENU_INIT0(
	294,44,0,137,		/* width, height, x, y */
	SAVESEG_MUID, 		/* id */
	PANELMENU,			/* type */
	&ssm_title_sel, 	/* buttons */
	SCREEN_FONT, 		/* font */
	&menu_cursor.hdr,	/* cursor */
	seebg_white, 		/* seebg */
	NULL,				/* dodata */
	NULL,				/* domenu */
	(MBPEN|MBRIGHT|KEYHIT), /* ioflags */
	0,				/* flags */
	NULL,			/* procmouse */
	NULL, 			/* on_showhide */
	NULL			/* cleanup */
);

static Smu_button_list sseg_smblist[] = {
	{ "title",  { &ssm_title_sel } },
	{ "save",   { &ssm_save_sel } },
	{ "cancel", { &ssm_cancel_sel } },
};

go_save_segment()
{
void *ss;

	hide_mp();
	if (soft_buttons("saveseg_panel", sseg_smblist, 
					 Array_els(sseg_smblist), &ss) < Success)
	{
		goto error;	
	}
	menu_to_cursor(vb.screen,&saveseg_menu);
	if((do_reqloop(vb.screen,&saveseg_menu,NULL,NULL,NULL)) >= Success)
		qsave_segment();
	smu_free_scatters(&ss);
error:
	show_mp();
}
