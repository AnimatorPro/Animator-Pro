#include "jimk.h"
#include "commonst.h"
#include "errcodes.h"
#include "ftextf.h"
#include "menus.h"
#include "softmenu.h"

static void see_screen_format(), cancel_flisize(), feel_numqdim(),
	set_full_screen(), see_savesize(), set_savesize();

static Button fmt_wslider_sel, fmt_hslider_sel; /* initialized later on */

static Rectangle flisize;

static Button fmt_ok_sel = MB_INIT1(
	NONEXT, /* next */
	NOCHILD, /* children */
	82, 11, 5, 43, /* w,h,x,y */
	NODATA, /* "OK", */
	dcorner_text,
	mb_gclose_ok,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);
static Button fmt_format_sel = MB_INIT1(
	&fmt_ok_sel, /* next */
	NOCHILD, /* children */
	143, 11, 91, 43, /* w,h,x,y */
	NODATA, /* "Full screen (![1] X ![2])",  */
	see_screen_format,
	set_full_screen,
	mb_gclose_identity,
	NOGROUP,RESET_SCREEN_SIZE,
	NOKEY,
	0 /* flags */
	);
static Button fmt_cancel_sel = MB_INIT1(
	&fmt_format_sel, /* next */
	NOCHILD, /* children */
	77, 11, 238, 43, /* w,h,x,y */
	NODATA, /* "Cancel", */
	dcorner_text,
	mb_gclose_cancel,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);

static Short_xy savesize;

static Button fmt_savesize_sel = MB_INIT1(
	&fmt_cancel_sel, /* next */
	NOCHILD, /* children */
	77, 11, 238, 27, /* w,h,x,y */
	NODATA, /* "![1] X ![2]", */ 
	see_savesize,
	set_savesize,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);

void update_slgroup(void *dat,Button *b)
{
	draw_buttontop(b->group);
}
static Qslider wslider = 
	QSL_INIT1( 4, 1200, (SHORT *)&flisize.width, 
			   0, update_slgroup, leftright_arrs);

static Qslider hslider = 
	QSL_INIT1( 4, 1200, (SHORT *)&flisize.height, 
			   0, update_slgroup, leftright_arrs);

static void feel_numqdim(Button *b)
{
#define VAL (*(SHORT *)(((Numq *)(b->datme))->val))

	feel_numq(b);
	if(VAL < 2)
	{
		softerr(Err_nogood, "!%d", "low_dim", 2);
		VAL = 2;
		draw_buttontop(b);
	}
	draw_button(b->children->group);

#undef VAL
}

#undef MINSIZE
static Numq wnumq = NUMQ_INIT(&flisize.width);
static Numq hnumq = NUMQ_INIT(&flisize.height);

static Button fmt_height_lab = MB_INIT1(
	NONEXT,
	NOCHILD, /* children */
	42, 11, 3, 27, /* w,h,x,y */
	NODATA, /* "Height", */
	black_label,
	NOFEEL,
	NOOPT,
	&fmt_hslider_sel,0,
	NOKEY,
	0 /* flags */
	);
static Button fmt_height_sel = MB_INIT1(
	&fmt_savesize_sel, /* next */
	&fmt_height_lab, /* children */
	42, 11, 45, 27, /* w,h,x,y */
	&hnumq, /* datme */
	see_numq,
	feel_numqdim,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);
static Button fmt_hslider_sel = MB_INIT1(
	&fmt_height_sel, /* next */
	NOCHILD, /* children */
	143, 11, 91, 27, /* w,h,x,y */
	&hslider, /* datme */
	see_qslider,
	feel_qslider,
	NOOPT,
	&fmt_height_sel,0,
	NOKEY,
	0 /* flags */
	);
	static Button fmt_width_lab = MB_INIT1(
		NONEXT,
		NOCHILD, /* children */
		42, 11, 3, 14, /* w,h,x,y */
		NODATA, /* "Width", */
		black_label,
		NOFEEL,
		NOOPT,
		&fmt_wslider_sel,0,
		NOKEY,
		0 /* flags */
		);
static Button fmt_width_sel = MB_INIT1(
	&fmt_hslider_sel, /* next */
	&fmt_width_lab, /* children */
	42, 11, 45, 14, /* w,h,x,y */
	&wnumq,		/* datme */
	see_numq,
	feel_numqdim,
	NOOPT,
	&fmt_height_sel,0,
	'\t',
	0 /* flags */
	);
static Button fmt_wslider_sel = MB_INIT1(
	&fmt_width_sel, /* next */
	NOCHILD, /* children */
	143, 11, 91, 14, /* w,h,x,y */
	&wslider, /* datme */
	see_qslider,
	feel_qslider,
	NOOPT,
	&fmt_width_sel,0,
	NOKEY,
	0 /* flags */
	);
static Button fmt_title_sel = MB_INIT1(
	&fmt_wslider_sel, /* next */
	NOCHILD, /* children */
	320, 10, 0, 0, /* w,h,x,y */
	NODATA, /* "Flic format menu", */
	see_titlebar,
	feel_titlebar,
	NOOPT,
	&tbg_moveclose,0,
	NOKEY,
	0 /* flags */
	);

static Menuhdr format_menu = {
	{320,60,0,0},
	FORMAT_MUID,	/* id */
	PANELMENU,		/* type */
	&fmt_title_sel,	/* buttons */
	SCREEN_FONT,	/* font */
	&menu_cursor,	/* cursor */
	seebg_white,	/* seebg */
	NULL,			/* data */
	NULL,			/* domenu */
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

static Smu_button_list fmt_smblist[] = {
	{ "title", &fmt_title_sel },
	{ "ok", &fmt_ok_sel },
	{ "fullsize", &fmt_format_sel },
	{ "cancel", &fmt_cancel_sel },
	{ "defaultsz", &fmt_savesize_sel },
	{ "hgt", &fmt_height_lab },
	{ "wid", &fmt_width_lab },
};

static void get_saved_settings()
{
Vset_flidef fdef;

	if(load_default_flidef(&fdef) >= Success)
	{
		savesize.x = fdef.rect.width;
		savesize.y = fdef.rect.height;
		enable_button(&fmt_savesize_sel);
	}
	else
		disable_button(&fmt_savesize_sel);
}
Errcode go_format_menu(Rectangle *outsize)
{
Errcode err;
void *ss;

	hide_mp();
	get_saved_settings();
	copy_rectfields(vb.pencel,&flisize);
	draw_flibord();

	if((err = soft_buttons("flisize_panel", fmt_smblist, 
					 Array_els(fmt_smblist), &ss)) < Success)
	{
		goto error;
	}
	if((err = do_reqloop(vb.screen,&format_menu,&fmt_width_sel,
						 NULL,NULL)) >= Success)
	{
		*outsize = flisize;
	}

	smu_free_scatters(&ss);
error:
	show_mp();
	erase_flibord();
	return(err);
}
static void see_twovals(Button *b, int one, int two)
{
char buf[80];
void *odat;

	odat = b->datme;
	snftextf(buf,sizeof(buf),"!%d%d", b->datme, one, two );
	b->datme = buf;
	dcorner_text(b);
	b->datme = odat;
}
static void see_screen_format(Button *b)
{
	see_twovals(b, vb.screen->wndo.width, vb.screen->wndo.height );
}
static void set_full_screen(Button *b)
{
	flisize.x = flisize.y = 0;
	flisize.width = vb.screen->wndo.width;
	flisize.height = vb.screen->wndo.height;
	mb_gclose_ok(b);
}
static void see_savesize(Button *b)
{
	if(b->flags & MB_DISABLED)
		white_block(b);
	else
		see_twovals(b,savesize.x, savesize.y );
}
static void set_savesize(Button *b)
{
	flisize.x = flisize.y = 0;
	flisize.width = savesize.x;
	flisize.height = savesize.y;
	mb_gclose_ok(b);
}
