/* This file handles the "join" menu.  It is the user interface for splicing
 * together animations.  See also composit.c for some of the actual machinery
 * for splicing.
 *
 * Peter Kennard wrote this file for Animator Pro.  Since then Jim Kent has
 * done some light maintenance work and commenting.
 */
#include <assert.h>
#include <stdio.h>
#include "jimk.h"
#include "commonst.h"
#include "composit.h"
#include "errcodes.h"
#include "flicel.h"
#include "flipath.h"
#include "flx.h"
#include "ftextf.h"
#include "memory.h"
#include "pentools.h"
#include "picdrive.h"
#include "picfile.h"
#include "rastcurs.h"
#include "softmenu.h"
#include "vsetfile.h"
#include "zoom.h"

static void qload_mask_cel(Button *b);
static void optload_mask_cel(Button *b);
static void qload_fli_b(Button *b);
static void see_fli_aname(Button *b);
static void see_fli_bname(Button *b);
static void see_mode_info(Button *b);
static void see_trans_icon(Button *b);
static void setup_nonmask_mode(Button *b);
static void setup_mask_mode(Button *b);
static void inc_overlap(Button *b);
static void tog_redraw_group(Button *b);
static void tog_ends(Button *b);
static void qrender_composite(Button *b);
static void see_abslid_sel(Button *b);
static void feel_abslid_sel(Button *b);
static void set_boxil_size(Button *b);
static void set_blindmask_mode(Button *b);
static void opt_blindmask_size(Button *b);
static int compos_get_overlap(void);

extern Image cright, cleft;		/* Left and right arrow icons. */

/**** icon button position values *****/

struct xycoors {
	SHORT unused_x;
	SHORT circ_diam;
	Short_xy vhwipe_oset;
	Short_xy blt_oset;
	Short_xy brb_oset;
	Rectangle box_rect;
	Rectangle hwed_rect;
	Rectangle vwed_rect;
	Short_xy blind_size;
	Short_xy poly_tl[3];
	Short_xy poly_bl[3];
	Short_xy poly_diamond[4];
};

#define IW 22 /* button inside width */
#define IH 13 /* button inside height */

static struct xycoors unscale_xys = {
	0,
	11,
	{ 7 - IW,3 - IH }, /* vhwipe_oset */
	{ 9 - IW,6 - IH }, /* blt_oset */
	{ IW - 9,IH - 6 }, /* brb_oset */
	{ 6,6,8,3 },
	{ 24,3,0,5 },
	{ 6,15,8,0 },
	{ 2,2 },
	{ { 0,0 }, {13,0}, {0,5} },
	{ { 0,IH }, {0,IH-6}, {14,IH} },
	{ { 0,IH/2 }, {IW/2,0}, {IW,IH/2}, {IW/2,IH} },
};

#undef IW
#undef IH

static struct xycoors scale_xys;

/******** end icon sizes ****/

/* The control with left and right arrows and overlapping slider bars. */
static Button seg_rarr_sel = MB_INIT1(
	NONEXT, /* next */
	NOCHILD, /* children */
	12, 20, 305, 15, /* w,h,x,y */
	&cright, /* datme */
	wbg_ncorner_image,
	inc_overlap,
	NOOPT,
	NOGROUP,1,
	NOKEY,
	0 /* flags */
	);
static Button seg_abslid_sel = MB_INIT1(
	&seg_rarr_sel, /* next */
	NOCHILD, /* children */
	190, 20, 116, 15, /* w,h,x,y */
	NODATA, /* datme */
	see_abslid_sel,
	feel_abslid_sel,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);
static Button seg_larr_sel = MB_INIT1(
	&seg_abslid_sel, /* next */
	NOCHILD, /* children */
	12, 20, 105, 15, /* w,h,x,y */
	&cleft, /* datme */
	wbg_ncorner_image,
	inc_overlap,
	NOOPT,
	NOGROUP,-1,
	NOKEY,
	0 /* flags */
	);

/* The canned wipe buttons. */
static Button seg_w14_sel = MB_INIT1(
	&seg_larr_sel, /* next */
	NOCHILD, /* children */
	24, 15, 293, 80, /* w,h,x,y */
	NULL, /* datme */
	see_trans_icon,
	setup_nonmask_mode,
	NOOPT,
	&vs.co_type,COMP_DIAMOND,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button seg_w13_sel = MB_INIT1(
	&seg_w14_sel, /* next */
	NOCHILD, /* children */
	24, 15, 268, 80, /* w,h,x,y */
	NULL, /* datme */
	see_trans_icon,
	setup_nonmask_mode,
	NOOPT,
	&vs.co_type,COMP_DIAGBOT,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button seg_w12_sel = MB_INIT1(
	&seg_w13_sel, /* next */
	NOCHILD, /* children */
	24, 15, 243, 80, /* w,h,x,y */
	NULL, /* datme */
	see_trans_icon,
	setup_nonmask_mode,
	NOOPT,
	&vs.co_type,COMP_DIAGTOP,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button seg_w11_sel = MB_INIT1(
	&seg_w12_sel, /* next */
	NOCHILD, /* children */
	24, 15, 218, 80, /* w,h,x,y */
	NULL, /* datme */
	see_trans_icon,
	set_blindmask_mode,
	opt_blindmask_size,
	&vs.co_type,COMP_LOUVER,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button seg_w10_sel = MB_INIT1(
	&seg_w11_sel, /* next */
	NOCHILD, /* children */
	24, 15, 193, 80, /* w,h,x,y */
	NULL, /* datme */
	see_trans_icon,
	set_blindmask_mode,
	opt_blindmask_size,
	&vs.co_type,COMP_VENETIAN,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button seg_w09_sel = MB_INIT1(
	&seg_w10_sel, /* next */
	NOCHILD, /* children */
	24, 15, 293, 64, /* w,h,x,y */
	NULL, /* datme */
	see_trans_icon,
	setup_nonmask_mode,
	NOOPT,
	&vs.co_type,COMP_BOXLBOT,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button seg_w08_sel = MB_INIT1(
	&seg_w09_sel, /* next */
	NOCHILD, /* children */
	24, 15, 268, 64, /* w,h,x,y */
	NULL, /* datme */
	see_trans_icon,
	setup_nonmask_mode,
	NOOPT,
	&vs.co_type,COMP_BOXRBOT,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button seg_w07_sel = MB_INIT1(
	&seg_w08_sel, /* next */
	NOCHILD, /* children */
	24, 15, 243, 64, /* w,h,x,y */
	NULL, /* datme */
	see_trans_icon,
	setup_nonmask_mode,
	NOOPT,
	&vs.co_type,COMP_HORIZW,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button seg_w06_sel = MB_INIT1(
	&seg_w07_sel, /* next */
	NOCHILD, /* children */
	24, 15, 218, 64, /* w,h,x,y */
	NULL, /* datme */
	see_trans_icon,
	setup_nonmask_mode,
	NOOPT,
	&vs.co_type,COMP_VWEDGE,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button seg_w05_sel = MB_INIT1(
	&seg_w06_sel, /* next */
	NOCHILD, /* children */
	24, 15, 193, 64, /* w,h,x,y */
	NULL, /* datme */
	see_trans_icon,
	setup_nonmask_mode,
	NOOPT,
	&vs.co_type,COMP_BOX,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button seg_w04_sel = MB_INIT1(
	&seg_w05_sel, /* next */
	NOCHILD, /* children */
	24, 15, 293, 48, /* w,h,x,y */
	NULL, /* datme */
	see_trans_icon,
	setup_nonmask_mode,
	NOOPT,
	&vs.co_type,COMP_BOXRTOP,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button seg_w03_sel = MB_INIT1(
	&seg_w04_sel, /* next */
	NOCHILD, /* children */
	24, 15, 268, 48, /* w,h,x,y */
	NULL, /* datme */
	see_trans_icon,
	setup_nonmask_mode,
	NOOPT,
	&vs.co_type,COMP_BOXLTOP,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button seg_w02_sel = MB_INIT1(
	&seg_w03_sel, /* next */
	NOCHILD, /* children */
	24, 15, 243, 48, /* w,h,x,y */
	NULL, /* datme */
	see_trans_icon,
	setup_nonmask_mode,
	NOOPT,
	&vs.co_type,COMP_VERTW,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button seg_w01_sel = MB_INIT1(
	&seg_w02_sel, /* next */
	NOCHILD, /* children */
	24, 15, 218, 48, /* w,h,x,y */
	NULL, /* datme */
	see_trans_icon,
	setup_nonmask_mode,
	NOOPT,
	&vs.co_type,COMP_HWEDGE,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button seg_w00_sel = MB_INIT1(
	&seg_w01_sel, /* next */
	NOCHILD, /* children */
	24, 15, 193, 48, /* w,h,x,y */
	NULL, /* datme */
	see_trans_icon,
	setup_nonmask_mode,
	NOOPT,
	&vs.co_type,COMP_CIRCLE,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button seg_boxil_sel = MB_INIT1(
	&seg_w00_sel, /* next */
	NOCHILD, /* children */
	59, 9, 131, 82, /* w,h,x,y */
	NODATA, /* "Boxilate", */
	wbg_ncorner_text,
	setup_nonmask_mode,
	set_boxil_size,
	&vs.co_type,COMP_BOXIL,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button seg_disol_sel = MB_INIT1(
	&seg_boxil_sel, /* next */
	NOCHILD, /* children */
	59, 9, 131, 72, /* w,h,x,y */
	NODATA, /* "Dissolve", */
	wbg_ncorner_text,
	setup_nonmask_mode,
	NOOPT,
	&vs.co_type,COMP_DISSOLVE,
	NOKEY,
	MB_B_GHILITE /* flags */
	);

static Button seg_mask_sel = MB_INIT1(
	&seg_disol_sel, /* next */
	NOCHILD, /* children */
	59, 9, 131, 62, /* w,h,x,y */
	NODATA, /* "Custom", */
	wbg_ncorner_text,
	setup_mask_mode,
	optload_mask_cel,
	&vs.co_type,COMP_MASK,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button seg_cut_sel = MB_INIT1(
	&seg_mask_sel, /* next */
	NOCHILD, /* children */
	59, 9, 131, 52, /* w,h,x,y */
	NODATA, /* "Cut", */
	wbg_ncorner_text,
	setup_nonmask_mode,
	NOOPT,
	&vs.co_type,COMP_CUT,
	NOKEY,
	MB_B_GHILITE /* flags */
	);

/*** Stuff to handle the column that asks what to do about the colors
 *** in overlapping frames. */
static void see_color_mode(Button *b)
/*-----------------------------------------------------------------------
 * Draw buttons for what to do about the colors in overlapping frames.
 * Since there are no overlapping frames if the transition is a simple
 * cut,  just draw in the background cover (to erase self if necessary)
 * in that case.
 *----------------------------------------------------------------------*/
{
	if(set_button_disable(b,vs.co_type == COMP_CUT))
		white_block(b);
	else
		ccorner_text(b);
}

static void see_color_label(Button *b)
/*-----------------------------------------------------------------------
 * Draw "color" label unless it's a CUT.
 *----------------------------------------------------------------------*/
{
	if(vs.co_type == COMP_CUT)
		white_block(b);
	else
		black_label(b);
}

	static Button seg_clabel_sel = MB_INIT1(
		NONEXT,
		NOCHILD, /* children */
		37, 9, 80, 50, /* w,h,x,y */
		NODATA, /* "Colors", */
		see_color_label,
		NOFEEL,
		NOOPT,
		NOGROUP,0,
		NOKEY,
		0 /* flags */
		);
	static Button seg_colab_sel = MB_INIT1(
		&seg_clabel_sel, /* next */
		NOCHILD, /* children */
		36, 11, 80, 80, /* w,h,x,y */
		NODATA, /* "Blend", */
		see_color_mode,
		change_mode,
		NOOPT,
		&vs.co_cfit,FIT_BLEND,
		NOKEY,
		MB_B_GHILITE /* flags */
		);
	static Button seg_colb_sel = MB_INIT1(
		&seg_colab_sel, /* next */
		NOCHILD, /* children */
		36, 11, 80, 70, /* w,h,x,y */
		NODATA, /* "2:", */
		see_color_mode,
		change_mode,
		NOOPT,
		&vs.co_cfit,FIT_TOB,
		NOKEY,
		MB_B_GHILITE /* flags */
		);
static Button seg_cola_sel = MB_INIT1(
	&seg_cut_sel, /* next */
	&seg_colb_sel, /* children */
	36, 11, 80, 60, /* w,h,x,y */
	NODATA, /* "1:", */
	see_color_mode,
	change_mode,
	NOOPT,
	&vs.co_cfit,FIT_TOA,
	NOKEY,
	MB_B_GHILITE /* flags */
	);

static Button seg_rev_sel = MB_INIT1(
	&seg_cola_sel, /* next */
	NOCHILD, /* children */
	70, 9, 5, 82, /* w,h,x,y */
	NODATA, /* "Reverse", */
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.co_reverse,1,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button seg_size_sel = MB_INIT1(
	&seg_rev_sel, /* next */
	NOCHILD, /* children */
	70, 9, 5, 72, /* w,h,x,y */
	NODATA, /* "Match Size", */
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.co_matchsize,1,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button seg_swap_sel = MB_INIT1(
	&seg_size_sel, /* next */
	NOCHILD, /* children */
	70, 9, 5, 62, /* w,h,x,y */
	NODATA, /* "Swap Ends", */
	ccorner_text,
	tog_ends,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	MB_HIONSEL /* flags */
	);
static Button seg_still_sel = MB_INIT1(
	&seg_swap_sel, /* next */
	NOCHILD, /* children */
	70, 9, 5, 52, /* w,h,x,y */
	NODATA, /* "Still", */
	ccorner_text,
	tog_redraw_group,
	NOOPT,
	&vs.co_still,1,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button seg_modeinfo_sel = MB_INIT1(
	&seg_still_sel, /* next */
	NOCHILD, /* children */
	320, 11, 2, 35, /* w,h,x,y */
	NULL, /* "Custom: ![1]  ![2] Frames, (![3] X ![4])", */
	see_mode_info,
	NOFEEL,
	qload_mask_cel,
	NOGROUP,0,
	NOKEY,
	MB_DISABOPT /* flags */
	);
static Button seg_nameb_sel = MB_INIT1(
	&seg_modeinfo_sel, /* next */
	NOCHILD, /* children */
	99, 11, 2, 25, /* w,h,x,y */
	NODATA, /* "2: ![1]", */
	see_fli_bname,
	NOFEEL,
	qload_fli_b,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);
static Button seg_namea_sel = MB_INIT1(
	&seg_nameb_sel, /* next */
	NOCHILD, /* children */
	99, 11, 2, 15, /* w,h,x,y */
	NODATA, /* "1: ![1]", */
	see_fli_aname,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);

static SHORT olapf;
static Numq frames_numq = NUMQ_INIT(&olapf);

static void see_olap_frames(Button *b)
{
	olapf = compos_get_overlap();
	see_numq(b);
}
static void feel_olap_frames(Button *b)
{
	feel_numq(b);
	vs.co_olap_frames = olapf;
	see_olap_frames(b);
	vs.co_olap_frames = olapf;
	draw_buttontop(&seg_abslid_sel);
}
static void opt_olap_frames(Button *b)
{
short maxtrans;

	if(vs.co_type == COMP_CUT)
		return;

	if(vs.co_still)
		maxtrans = 100;
	else
		maxtrans = Min(flix.hdr.frame_count,ccb.fcelb->flif.hdr.frame_count);

	olapf = compos_get_overlap();
	if(soft_qreq_number(&olapf,1,maxtrans,"comp_frames"))
	{
		vs.co_olap_frames = olapf;
		see_olap_frames(b);
		vs.co_olap_frames = olapf; /* put in clipped value */
		draw_buttontop(&seg_abslid_sel);
	}
}
static Button seg_frames_sel = MB_INIT1(
	&seg_namea_sel, /* next */
	NOCHILD, /* children */
	35, 11, 282, 2, /* w,h,x,y */
	&frames_numq, /* datme */
	see_olap_frames,
	feel_olap_frames,
	opt_olap_frames,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);
static Button seg_frlab_sel = MB_INIT1(
	&seg_frames_sel, /* next */
	NOCHILD, /* children */
	49, 9, 231, 3, /* w,h,x,y */
	NODATA, /* "Frames:", */
	black_label,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);

static Button seg_preview_sel = MB_INIT1(
	&seg_frlab_sel, /* next */
	NOCHILD, /* children */
	59, 9, 169, 3, /* w,h,x,y */
	NODATA, /* "Preview", */
	ccorner_text,
	qrender_composite,
	NOOPT,
	NOGROUP,1, /* one for preview */
	NOKEY,
	0 /* flags */
	);
static Button seg_render_sel = MB_INIT1(
	&seg_preview_sel, /* next */
	NOCHILD, /* children */
	59, 9, 105, 3, /* w,h,x,y */
	NODATA, /* "Render", */
	ccorner_text,
	qrender_composite,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);
static Button seg_title_sel = MB_INIT1(
	&seg_render_sel, /* next */
	NOCHILD, /* children */
	94, 9, 4, 3, /* w,h,x,y */
	NODATA, /* "Join", */
	see_titlebar,
	feel_titlebar,
	mb_menu_to_bottom,
	&tbg_moveclose,0,
	'q',
	0 /* flags */
	);

static Menuhdr seg_menu = MENU_INIT0(
	320,98,0,0,		/* x,y,w,h */
	COMPOS_MUID,	/* id */
	PANELMENU,		/* type */
	&seg_title_sel,	/* buttons */
	SCREEN_FONT,	/* font */
	&menu_cursor.hdr, /* cursor */
	seebg_white,	/* seebg */
	NULL,			/* data */
	NULL,			/* domenu */
	(MBPEN|MBRIGHT|KEYHIT),	/* ioflags */
	0,				/* flags */
	NULL,			/* procmouse */
	NULL,			/* on_showhide */
	NULL			/* cleanup */
);


static Smu_button_list co_smblist[] = {
	{ "title",      { &seg_title_sel } },
	{ "name1",      { &seg_namea_sel } },
	{ "name2",      { &seg_nameb_sel } },
	{ "boxilate",   { &seg_boxil_sel } },
	{ "dissolve",   { &seg_disol_sel } },
	{ "custom",     { &seg_mask_sel } },
	{ "cut",        { &seg_cut_sel } },
	{ "color",      { &seg_clabel_sel } },
	{ "blend",      { &seg_colab_sel } },
	{ "col2",       { &seg_colb_sel } },
	{ "col1",       { &seg_cola_sel } },
	{ "rev",        { &seg_rev_sel } },
	{ "match",      { &seg_size_sel } },
	{ "swapend",    { &seg_swap_sel } },
	{ "still",      { &seg_still_sel } },
	{ "frames",     { &seg_frlab_sel } },
	{ "preview",    { &seg_preview_sel } },
	{ "rend",       { &seg_render_sel } },
	{ "custinfo",   { &seg_modeinfo_sel } },
};

Errcode reload_mask_cel(void)
{
Errcode err;

	if(ccb.mask_cel != NULL
		&& flipaths_same(ccb.maskpath,ccb.mask_cel->cpath))
	{
		return(Success);
	}

	if((err = pdr_load_any_flicel(ccb.maskpath->path,NULL,NULL,&ccb.mask_cel))
		< Success)
	{
		goto error;
	}
	if(!flipaths_same(ccb.maskpath,ccb.mask_cel->cpath))
	{
		err = Err_invalid_id;
		goto error;
	}
	return(Success);
error:
	free_fcel(&ccb.mask_cel);
	return(softerr(err,"!%s", "join_remask",ccb.maskpath->path));
}
static Errcode load_mask_cel(char *path)
{
Errcode err;

	if((err = pdr_load_any_flicel(path, NULL, NULL, &ccb.mask_cel)) < Success)
		ccb.maskpath->path[0] = 0;
	else
		copy_flipath(ccb.mask_cel->cpath,ccb.maskpath);
	return(err);
}
static Errcode ask_load_mask_cel(void)
{
Errcode err;
char path[PATH_SIZE];
char sbuf[50];
char suffi[16];

	hide_mp();
	sprintf(suffi,".FL?;.CEL;%.4s", get_pictype_suffi());

	for(;;)
	{
		if(vset_get_filename(stack_string("load_fli_mask",sbuf),
						   suffi, load_str,JOIN_MASK_PATH,
						   path,0) == NULL)
		{
			err = Err_abort;
			break;
		}
		if((err = load_mask_cel(path)) < Success)
		{
			softerr(err,"!%s", "join_mask", path);
			continue;
		}
		break;
	}

	show_mp();
	return(err);
}
static void qload_mask_cel(Button *b)
{
	ask_load_mask_cel();
	draw_buttontop(b);
}
static void optload_mask_cel(Button *b)
{
	if(vs.co_type == COMP_MASK)
		qload_mask_cel(b);
	else
		setup_mask_mode(b);
}
static Errcode load_fli_b(void)
{
Errcode err;
char suffi[PDR_SUFFI_SIZE*2 +10];
char path[PATH_SIZE];
char header[80];
char hdtext[60];

	hide_mp();

	sprintf(suffi,".FL?;.CEL;%.9s", get_pictype_suffi());
	stack_string("load_flic2", hdtext);

	for(;;)
	{
		snftextf(header,sizeof(header),
				"!%s",hdtext,pj_get_path_name(ccb.tflxpath->path));
		
		if(vset_get_filename(header,suffi,load_str,JOIN_PATH,path,0) == NULL)
		{
			err = Err_abort;
			break;
		}

		/* note: this call will free the old cel if it loads a new one */

		if((err = pdr_load_any_flicel(path, NULL, NULL, &ccb.fcelb)) < Success)
			goto error;

		/* Make sure there is a flipath record, if we loaded a pic there won't
		 * be any path, Join menu and this loop needs the path */

		if(ccb.fcelb->cpath != NULL)
			break;

		if((err = alloc_flipath(path,&ccb.fcelb->flif,
								&ccb.fcelb->cpath)) < Success)
		{
			free_fcel(&ccb.fcelb);
			goto error;
		}
		break;

	error:
		softerr(err,"!%s", "join_load", path);
		continue;
	}


	show_mp();
	return(err);
}
static void qload_fli_b(Button *b)
{
Errcode err;

	hide_mp();
	if(((err = load_fli_b()) < Success) && !ccb.fcelb)
		mb_gclose_code(b,err);
	else
		show_mp();
}

static void cleanup_composite(void)
{
	free_fcel(&ccb.fcela);
	free_fcel(&ccb.fcelb);
	free_fcel(&ccb.mask_cel);
	pj_freez(&ccb.tflxpath);
}
static Errcode init_paths_and_cels(void)
{
	if(NULL == (ccb.tflxpath = pj_malloc(sizeof(Flipath)*2)))
		return(Err_no_memory);
	ccb.maskpath = ccb.tflxpath + 1;
	ccb.maskpath->path[0] = 0;
	if(vs.co_type == COMP_MASK)
	{
		if(vset_get_path(JOIN_MASK_PATH, ccb.maskpath->path) < Success
			|| load_mask_cel(ccb.maskpath->path) < Success)
		{
			vs.co_type = COMP_CUT;
		}
	}
	return(read_flx_path(&flix,ccb.tflxpath));
}
static void scale_seg_menu(Rscale *scale)
{
	scale_xylist(scale, (Short_xy *)&unscale_xys, (Short_xy *)&scale_xys,
					sizeof(scale_xys)/sizeof(Short_xy));
}
static void go_seg_menu(void)
{
Errcode err;
void *ss;

	if((err = soft_buttons("join_panel", co_smblist, Array_els(co_smblist),
							&ss )) < Success)
	{
		goto early_error;
	}

	if((err = init_paths_and_cels()) < Success)
		goto error;

	scale_seg_menu(&vb.screen->menu_scale);

	if((err = load_fli_b()) < Success) /* ask user for fli b */
		goto error;

	menu_to_quickcent(&seg_menu);
	err = do_reqloop(vb.screen,&seg_menu,NULL,NULL,check_pen_abort);
error:
	cleanup_composite();
early_error:
	smu_free_scatters(&ss);
	softerr(err,"join_menu");
	add_check_tflx_toram();
	return;
}
void qdo_composite(void)
{
	hide_mp();
	unzoom();
	push_most();
	scrub_cur_frame();
	pj_delete(screen_name); /* we need the space */
	pj_delete(bscreen_name); /* we need the space */
	go_seg_menu();
	pop_most();
	rezoom();
	show_mp();
}

static void see_celname(Button *b, Flipath *fpath)
{
char buf[32];
void *odat;

	snftextf(buf,sizeof(buf), "!%s", b->datme, pj_get_path_name(fpath->path));
	odat = b->datme;
	b->datme = buf;
	black_leftlabel(b);
	b->datme = odat;
}
static void see_fli_aname(Button *b)
{
	see_celname(b,ccb.tflxpath);
}
static void see_fli_bname(Button *b)
{
	see_celname(b,ccb.fcelb->cpath);
}
static void see_mode_info(Button *b)
/*-----------------------------------------------------------------------
 * Show the current mask flic if any.
 *----------------------------------------------------------------------*/
{
char buf[80];
void *odat;

	if(set_button_disable(b,vs.co_type != COMP_MASK)
		|| ccb.maskpath->path[0] == 0
		|| ccb.mask_cel == NULL )
	{
		white_block(b);
		return;
	}

	odat = b->datme;
	snftextf(buf, sizeof(buf), "!%.14s%d%d%d",  b->datme,
			pj_get_path_name(ccb.maskpath->path),
			ccb.mask_cel->flif.hdr.frame_count,
			ccb.mask_cel->flif.hdr.width,
			ccb.mask_cel->flif.hdr.height );

	b->datme = buf;
	black_ltext(b);
	b->datme = odat;
}

static void see_trans_icon(Button *b)
/* see transition icon button */
{
Pixel icolor;
Short_xy cent;
Rectangle rect;
Clipbox cb;
SHORT max, inc;
SHORT *ival;
Short_xy *poly;

	wbg_ncorner_back(b);
	icolor = wbg_textcolor(b);
	mb_make_iclip(b,&cb);
	rect.x = rect.y = 0;
	rect.width = cb.width;
	rect.height = cb.height;
	cent.x = cb.width>>1;
	cent.y = cb.height>>1;

	switch(b->identity)
	{
		case COMP_CIRCLE:
			circle(&cb,icolor,cent.x,cent.y,scale_xys.circ_diam,TRUE);
			break;
		case COMP_VERTW:
			rect.x = scale_xys.vhwipe_oset.x;
			goto draw_rect;
		case COMP_HORIZW:
			rect.y = scale_xys.vhwipe_oset.y;
			goto draw_rect;
		case COMP_BOXLTOP:
			rect.x = scale_xys.blt_oset.x;
			rect.y = scale_xys.blt_oset.y;
			goto draw_rect;
		case COMP_BOXRTOP:
			rect.x = scale_xys.brb_oset.x;
			rect.y = scale_xys.blt_oset.y;
			goto draw_rect;
		case COMP_BOXRBOT:
			rect.x = scale_xys.brb_oset.x;
			rect.y = scale_xys.brb_oset.y;
			goto draw_rect;
		case COMP_BOXLBOT:
			rect.x = scale_xys.blt_oset.x;
			rect.y = scale_xys.brb_oset.y;
			goto draw_rect;
		case COMP_HWEDGE:
			rect = scale_xys.hwed_rect;
			goto draw_rect;
		case COMP_VWEDGE:
			rect = scale_xys.vwed_rect;
			goto draw_rect;
		case COMP_BOX:
			rect = scale_xys.box_rect;
		draw_rect:
			pj_set_rect(&cb,icolor,rect.x,rect.y,rect.width,rect.height);
			break;

		case COMP_VENETIAN:
			inc = rect.height = scale_xys.blind_size.y;
			ival = &rect.y;
			max = cb.height;
			goto do_blinds;

		case COMP_LOUVER:
			inc = rect.width = scale_xys.blind_size.x;
			ival = &rect.x;
			max = cb.width;

		do_blinds:

			for(inc<<=1;*ival < max;*ival += inc)
				pj_set_rect(&cb,icolor,rect.x,rect.y,rect.width,rect.height);

			break;

		case COMP_DIAGTOP:
			poly = scale_xys.poly_tl;
			goto do_triang;
		case COMP_DIAGBOT:
			poly = scale_xys.poly_bl;
		do_triang:
			inc = 3;
			goto do_poly;
		case COMP_DIAMOND:
			scale_xys.poly_diamond[2].x = cb.width - 1;
			scale_xys.poly_diamond[3].y = cb.height - 1;
			poly = scale_xys.poly_diamond;
			inc = 4;
		do_poly:
			polygon(&cb,icolor,poly,inc,TRUE);
			break;
		default:
			break;
	}
}
static void change_trans_mode(Button *b)
/* change transition mode */
{
Boolean is_cut;

	change_mode(b);
	is_cut = (vs.co_type == COMP_CUT);
	draw_button_disable(&seg_rev_sel, is_cut);
	draw_button_disable(&seg_still_sel, is_cut);
	draw_buttontop(&seg_abslid_sel);
	draw_buttontop(&seg_frames_sel);
	draw_button(&seg_cola_sel);
	draw_buttontop(&seg_modeinfo_sel);
}
static void setup_nonmask_mode(Button *b)
{
	free_fcel(&ccb.mask_cel);
	change_trans_mode(b);
}
static void setup_mask_mode(Button *b)
/* change transition mode to one that uses a mask cel */
{
	if(!ccb.mask_cel)
	{
		if(ccb.maskpath->path[0] == 0 || ccb.maskpath->fid.create_time == 0
			|| reload_mask_cel() < Success)
		{
			ask_load_mask_cel();
		}
	}
	if(ccb.mask_cel)
		change_trans_mode(b);
}
static void incit(void *b)
{
	if(vs.co_type != COMP_CUT)
		vs.co_olap_frames += ((Button *)b)->identity;
	draw_buttontop(&seg_abslid_sel);  /* this will clip things */
	draw_buttontop(&seg_frames_sel);
}
static void inc_overlap(Button *b)
{
	hilight(b);
	repeat_on_pdn(incit,b);
	draw_buttontop(b);
}
static int compos_get_overlap(void)
/* returns clipped value for current mode does not alter global setting */
{
int maxolap;
int olap;

	maxolap = ccb.fcelb->flif.hdr.frame_count;
	if(vs.co_still)
		maxolap = MAXFRAMES - (maxolap + flix.hdr.frame_count + 1);
	else
		maxolap = Min(flix.hdr.frame_count,maxolap);

	olap = vs.co_olap_frames;
	if(olap > maxolap)
		olap = maxolap;
	else if(olap <= 0)
		olap = 1;

	if(vs.co_type == COMP_CUT) /* 0 transition is a cut by definition */
		return(0);
	return(olap);
}
static void tog_redraw_group(Button *b)
{
	toggle_bgroup(b);
	draw_buttontop(&seg_abslid_sel);
	draw_buttontop(&seg_frames_sel);
}
static void tog_ends(Button *b)
{
	(void)b;

	vs.co_b_first = !vs.co_b_first;
	draw_buttontop(&seg_abslid_sel);
}
static void qrender_composite(Button *b)
{
Boolean preview;

	preview = b->identity != 0;
	hide_mp();
	vs.co_olap_frames = ccb.transition_frames = compos_get_overlap();
	if((render_composite(preview) >= Success && !preview)
		|| ccb.fcelb == NULL )
	{
		mb_gclose_ok(b);
	}
	else
	{
		if(vs.co_type == COMP_MASK)
			reload_mask_cel();
		show_mp();
	}
}

/****** stuff for abslider *******/

typedef struct abslidat {
	Clipbox cb;
	int totpix, totframes;
	int olap_start, olap_wid;
	int astart, awid;
	int bstart, bwid;
} Abslidat;

static void build_abslid(Button *b, Abslidat *abs)
{
int olap_frames;
int olap_start, olap_wid;
int astart, awid;
int bstart, bwid;
int totpix;
int totframes;

	mb_make_iclip(b,&abs->cb);
	olap_frames = compos_get_overlap();
	totframes = flix.hdr.frame_count + ccb.fcelb->flif.hdr.frame_count;
	if(vs.co_still)
		totframes += olap_frames;

	totpix = abs->cb.width - 2; 
	awid = pj_uscale_by(totpix,flix.hdr.frame_count,totframes);
	bwid = pj_uscale_by(totpix,ccb.fcelb->flif.hdr.frame_count,totframes);

	if(bwid == 0)
	{
		++bwid;
		--awid;
	}
	else if(awid == 0)
	{
		++awid;
		--bwid;
	}
	olap_wid = pj_uscale_by(totpix,olap_frames,totframes);
	if(olap_frames && !olap_wid)
		++olap_wid;

	if(vs.co_still)
	{
		if(vs.co_b_first)
		{
			bstart = 1;
			olap_start = 1 + bwid;
			astart = olap_start + olap_wid;
		}
		else
		{
			astart = 1;
			olap_start = 1 + awid;
			bstart = olap_start + olap_wid;
		}
	}
	else
	{
		bstart = 1 + (olap_wid>>1);
		if(vs.co_b_first)
		{
			astart = bstart + bwid - olap_wid;
			olap_start = astart;
		}
		else
		{
			astart = bstart;
			bstart = astart + awid - olap_wid;
			olap_start = bstart;
		}
	}

	abs->totframes = totframes;
	abs->totpix = totpix;
	abs->olap_start = olap_start;
	abs->olap_wid = olap_wid;
	abs->astart = astart;
	abs->awid = awid;
	abs->bstart = bstart;
	abs->bwid = bwid;
}
static void build_draw_abslid(Button *b, Abslidat *abs)
{
Pixel white;
Pixel black;
Short_xy cent;

	build_abslid(b,abs);
	mc_frame(b,MC_GREY);
	cent.x = abs->cb.width>>1;
	cent.y = abs->cb.height>>1;

	white = mc_white(b);
	black = mc_black(b);

	pj_set_rect(&abs->cb,white,0,0,abs->cb.width,cent.y);
	pj_set_rect(&abs->cb,black,abs->astart,1,abs->awid,cent.y-1);
	pj_set_rect(&abs->cb,white,0,cent.y,abs->cb.width,abs->cb.height);
	pj_set_rect(&abs->cb,black,abs->bstart,cent.y,
			 abs->bwid,abs->cb.height-cent.y-1);

	if(abs->olap_wid)
	{
		pj_set_rect(&abs->cb,mc_grey(b),abs->olap_start,abs->cb.height>>2,
				 abs->olap_wid,abs->cb.height>>1);
	}
}
static void see_abslid_sel(Button *b)
{
Abslidat abs;

	build_draw_abslid(b,&abs);
}
static void feel_abslid_sel(Button *b)
{
Abslidat abs;
Short_xy cent;
SHORT omx, dmx;
Boolean top;
int olapsize;
int oolap, lastolap;

	if(vs.co_type == COMP_CUT)
		return;

	build_abslid(b,&abs);
	cent.x = abs.cb.width>>1;
	cent.y = abs.cb.height>>1;
	top = (icb.my - b->y) < cent.y;

	olapsize = Min(flix.hdr.frame_count,ccb.fcelb->flif.hdr.frame_count);

	if((oolap = compos_get_overlap()) > olapsize)
		olapsize = Min(100,oolap);

	omx = icb.mx;

	while(ISDOWN(MBPEN))
	{
		wait_input(MBPUP|MMOVE);
		dmx = icb.mx - omx;
		if(!top)
			dmx = -dmx;
		if(vs.co_b_first)
			dmx = -dmx;
		if(vs.co_still)
			dmx = -dmx;


		lastolap = compos_get_overlap();
		vs.co_olap_frames = oolap + rscale_by(olapsize,dmx*2,b->width - 4);
		if((vs.co_olap_frames = compos_get_overlap()) != lastolap)
			build_draw_abslid(b,&abs);
		draw_buttontop(&seg_frames_sel);
	}
}
struct boxil_zdata {
	Rcel *zdots;
	Boolean size_is_height;
};
static Errcode zoom_boxils(void *boxil_zdata, SHORT size)
{
	struct boxil_zdata *bd = boxil_zdata;
	LONG hsize;
	LONG vsize;
	assert(size >= 0);

	if(bd->size_is_height)
	{
		vsize = size;
		hsize = vs.co_boxil_width;
	}
	else
	{
		vsize = vs.co_boxil_height;
		hsize = size;
	}
	zoom_boxil_mask((Raster *)(bd->zdots),(Raster *)vb.pencel,hsize,vsize);
	return(Success);
}
static void set_boxil_size(Button *b)
{
Errcode err;
SHORT *size;
SHORT maxsize;
SHORT osize;
Pixel *hsegbuf = NULL;
int i;
struct boxil_zdata bd;
char *key;

	setup_nonmask_mode(b);

	hide_mp();
	save_undo();

	if((err =  alloc_pencel(&bd.zdots)) < Success)
		goto error;

	if((err = ealloc(&hsegbuf, bd.zdots->width + 4)) < Success)
		goto error;

	/* stuff zdots with checkered dots in two colors */
	pj_stuff_words(((sgrey&0xFF)|(swhite<<8)),hsegbuf,(bd.zdots->width + 4)>>1);

	for(i = 0;i < bd.zdots->height;++i)
		pj_put_hseg(bd.zdots, hsegbuf + (i&1), 0, i, bd.zdots->width);

	pj_freez(&hsegbuf);

	key = "boxil_ht";
	size = &vs.co_boxil_height;
	bd.size_is_height = TRUE;
	maxsize = vb.pencel->height/3;

	for(;;)
	{
		zoom_boxils(&bd,*size);
		osize = *size;
		++maxsize;

		for(;;)
		{
			if(!soft_ud_qreq_number(size,1,maxsize,zoom_boxils,&bd,key))
			{
				*size = osize;
				break;
			}
			if(*size >= 1 && *size <= (maxsize<<1))
				break;
			softerr(Err_nogood,"!%d%d","outa_range", 1, maxsize<<1);
			*size = osize;
		}

		if(!bd.size_is_height)
			break;

		key = "boxil_wid";
		size = &vs.co_boxil_width;
		maxsize = vb.pencel->width/3;
		bd.size_is_height = FALSE;
	}

	err = Success;
error:
	pj_rcel_free(bd.zdots);
	pj_freez(&hsegbuf);
	softerr(err,"join_boxsize");
	zoom_unundo();
	show_mp();
}
static void set_blindmask_size(Button *b)
{
SHORT osize;
SHORT *size;
SHORT maxsize;
Boolean vertical;

	vertical = (b->identity == COMP_LOUVER);

	hide_mp();
	if(vertical)
	{
		size = &vs.co_louver_width;
		maxsize = vb.pencel->width/2;
	}
	else
	{
		size = &vs.co_venetian_height;
		maxsize = vb.pencel->height/2;
	}

	osize = *size;
	save_undo();
	for(;;)
	{
		draw_slatmask(&vertical,*size);
		if(!soft_ud_qreq_number(size,2,maxsize,draw_slatmask,
								&vertical,"slatsize"))
		{
			*size = osize;
		}
		if(*size >= 2 && *size <= (maxsize<<1))
			break;
		softerr(Err_nogood,"!%d%d","outa_range", 2, (maxsize<<1));
		*size = 2;
	}
	zoom_unundo();
	show_mp();
}
static void set_blindmask_mode(Button *b)
{
	if(vs.co_type == b->identity)
		set_blindmask_size(b);
	else
		setup_nonmask_mode(b);
}
static void opt_blindmask_size(Button *b)
{
	setup_nonmask_mode(b);
	set_blindmask_size(b);
}
