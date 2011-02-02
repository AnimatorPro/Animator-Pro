
/* multimen.c - Data structures and code associated with the 'Time Select'
   menu that gets called by almost anything that does something over
   move than one frame.  Basic argument to this module is an 'auto-vec'
   which will produce a transformed render_form given the original
   render form and an up-to-date uf.  (undo buffer).  This guy lets
   you select the part of your FLIC you want to transform, specify
   ease in/ease out, go visit the palette editor, do a pixel perfect
   (but slow) preview, and finally actually call do_auto with the 
   auto-vec. */

#include "jimk.h"
#include "flicmenu.h"
#include "commonst.h"
#include "multimen.str"

extern dcorner_text(), ccorner_text(), ncorner_text(), gary_menu_back(),
	ccolor_box(), ppalette(), see_pen(), toggle_pen(), set_pbrush(),
	minks(), see_cur_ink(), force_opaque(),
	move_tab_text(), move_menu(), bottom_menu(),
	dcorner_text(), mgo_stencil(), change_time_mode(),
	multi_preview(), multi_use(), close_menu(),
	mum_menu_back(), see_mask_m(), toggle_mask(),
	hang_child(), toggle_group(), change_mode();

extern Flicmenu minitime_sel, tseg_group_sel, quick_menu;


/*** Button Data ***/
static Flicmenu mum_toa_sel = {
	NONEXT,
	NOCHILD,
	244, 185, 66, 10,
	multimen_100 /* "to all" */,
	ccorner_text,
	change_time_mode,
	&vs.time_mode, 2,
	NOKEY,
	NOOPT,
	};
static Flicmenu mum_tos_sel = {
	&mum_toa_sel,
	NOCHILD,
	244, 172, 66, 10,
	multimen_101 /* "to segment" */,
	ccorner_text,
	change_time_mode,
	&vs.time_mode, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu mum_tof_sel = {
	&mum_tos_sel,
	NOCHILD,
	244, 159, 66, 10,
	multimen_102 /* "to frame" */,
	ccorner_text,
	change_time_mode,
	&vs.time_mode, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu mum_com_sel = {
	&mum_tof_sel,
	NOCHILD,
	160, 185, 53, 10,
	multimen_103 /* "complete" */,
	ccorner_text,
	toggle_group,
	&vs.ado_complete, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu mum_out_sel = {
	&mum_com_sel,
	NOCHILD,
	160, 172, 53, 10,
	multimen_104 /* "Out Slow" */,
	ccorner_text,
	toggle_group,
	&vs.ado_ease_out,1,
	NOKEY,
	NOOPT,
	};
static Flicmenu mum_ins_sel = {
	&mum_out_sel,
	NOCHILD,
	160, 159, 53, 10,
	multimen_105 /* "In Slow" */,
	ccorner_text,
	toggle_group,
	&vs.ado_ease, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu mum_rev_sel = {
	&mum_ins_sel,
	NOCHILD,
	88, 185, 59, 10,
	multimen_106 /* "reverse" */,
	ccorner_text,
	toggle_group,
	&vs.ado_reverse, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu mum_pin_sel = {
	&mum_rev_sel,
	NOCHILD,
	88, 172, 59, 10,
	multimen_107 /* "ping-pong" */,
	ccorner_text,
	toggle_group,
	&vs.ado_pong, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu mum_sti_sel = {
	&mum_pin_sel,
	NOCHILD,
	88, 159, 59, 10,
	multimen_108 /* "still" */,
	ccorner_text,
	toggle_group,
	&vs.ado_tween, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu mum_ok_sel = {
	&mum_sti_sel,
	NOCHILD,
	9, 185, 48, 10,
	multimen_109 /* "render" */,
	dcorner_text,
	multi_use,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu mum_pre_sel = {
	&mum_ok_sel,
	NOCHILD,
	9, 172, 48, 10,
	multimen_110 /* "preview" */,
	dcorner_text,
	multi_preview,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu mum_can_sel = {
	&mum_pre_sel,
	NOCHILD,
	9, 159, 48, 10,
	cst_cancel,
	dcorner_text,
	close_menu,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu mum_tse_sel = {
	&mum_can_sel,
	&tseg_group_sel,
	4, 145, 311, 10,
	NOTEXT,
	hang_child,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu mum_cco_sel = {
	&mum_tse_sel,
	NOCHILD,
	305, 133, 10, 8,
	NOTEXT,
	ccolor_box,
	ppalette,
	NOGROUP, 0,
	NOKEY,
	ppalette,
	};
static Flicmenu mum_bru_sel = {
	&mum_cco_sel,
	NOCHILD,
	288, 132, 10, 10,
	NOTEXT,
	see_pen,
	toggle_pen,
	NOGROUP, 0,
	NOKEY,
	set_pbrush,
	};
static Flicmenu mum_kmo_sel = {
	&mum_bru_sel,
	NOCHILD,
	272, 133, 10, 8,
	multimen_112 /* "k" */,
	ccorner_text,
	toggle_group,
	&vs.zero_clear, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu mum_smo_sel = {
	&mum_kmo_sel,
	NOCHILD,
	257, 133, 10, 8,
	NULL,
	see_mask_m,
	toggle_mask,
	&vs.use_mask, 1,
	NOKEY,
	mgo_stencil,
	};
static Flicmenu mum_fmo_sel = {
	&mum_smo_sel,
	NOCHILD,
	242, 133, 10, 8,
	multimen_114 /* "f" */,
	ccorner_text,
	toggle_group,
	&vs.fillp, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu mum_mmo_sel = {
	&mum_fmo_sel,
	NOCHILD,
	227, 133, 10, 8,
	multimen_115 /* "T" */,
	ccorner_text,
	toggle_group,
	&vs.multi, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu mum_ink_sel = {
	&mum_mmo_sel,
	NOCHILD,
	184, 133, 38, 8,
	NOTEXT,
	see_cur_ink,
	force_opaque,
	&vs.draw_mode,0,
	NOKEY,
	minks,
	};
static Flicmenu mum_min_sel = {
	&mum_ink_sel,
	&minitime_sel,
	104, 133, 77, 8,
	NOTEXT,
	hang_child,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu mum_tit_sel = {
	&mum_min_sel,
	NOCHILD,
	4, 133, 91, 8,
	multimen_116 /* "Time Select" */,
	move_tab_text,
	move_menu,
	NOGROUP, 0,
	NOKEY,
	bottom_menu,
	};
static Flicmenu mum_menu = {
	NOCHILD,
	&mum_tit_sel,
	0, 130, 319, 69,
	NOTEXT,
	gary_menu_back,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};

static Vector multivec;

static
multi_use()
{
if (multivec != NULL)
	{
	hide_mp();
	dauto(multivec, vs.time_mode);
	draw_mp();
	}
close_menu();
}

static
multi_preview()
{
if (multivec != NULL)
	{
	hide_mp();
	dopreview(multivec);
	draw_mp();
	}
}


multimenu(v)
Vector v;
{
static in_multi = 0;

if (!in_multi)	/* make sure don't recurse back into self */
	{
	in_multi = 1;
	mum_pre_sel.disabled = mum_ok_sel.disabled = (v == NULL);
	multivec = v;
	clip_tseg();
	clip_rmove_menu(&mum_menu, 
		quick_menu.x - mum_menu.x, quick_menu.y-mum_menu.y); 
	do_menu(&mum_menu);
	in_multi = 0;
	}
else
	{
	go_in_circles_message(multimen_117 /* "time select" */);
	}
}


go_multi()
{
hide_mp();
multimenu((Vector)NULL);
draw_mp();
}

