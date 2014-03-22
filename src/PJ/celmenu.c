#include "jimk.h"
#include "broadcas.h"
#include "celmenu.h"
#include "errcodes.h"
#include "flx.h"
#include "inks.h"
#include "rastrans.h"
#include "render.h"
#include "softmenu.h"

/* globals for cel menu functions */

Celmu_cb *cmcb;

/* seeme feelme */

extern void qinks(), see_cur_ink();

/* locals */

static Minitime_data celtime_data;
static Minitime_data cm_flitime_data;

static void see_cel_minitime(Button *b);
static void mb_set_celtool(Button *b);
static void exit_refresh_cel(Pentool *pt);
static Errcode cel_scale_ptfunc(Pentool *pt, Wndo *w);
static Errcode cel_rotate_ptfunc(Pentool *pt, Wndo *w);
static Errcode cel_move_ptfunc(Pentool *pt, Wndo *w);
static Errcode init_tcolor_ptool(Pentool *pt);
static Errcode cel_tcolor_ptfunc(Pentool *pt, Wndo *w);

/******* button sub groups for tools ****/

/***** paint (and) paste group sel *****/

static Button cmu_bluelast_sel = MB_INIT1(
	NONEXT,
	NOCHILD, 		/* children */
	80, 9, 0,0, 	/* w,h,x,y */
	NODATA, /* "Blue Last", */
	ccorner_text,   /* seeme */
	toggle_bgroup,  /* feelme */
	NOOPT,
	&vs.cm_blue_last, 1,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static void hang_bluelast(Button *b)
{
	/* only if paint tool and not streamdraw */
	b->children = &cmu_bluelast_sel; /* always hang children to do offsets */
	hang_children(b);
	if((vs.cur_cel_tool != CELPT_PAINT) || vs.cm_streamdraw)
	{
		b->children = NULL;
		white_block(&cmu_bluelast_sel); /* clear where it would be */
	}
}
static Button cmu_bluelast_hanger = MB_INIT1(
	NONEXT,
	&cmu_bluelast_sel,	/* children */
	80, 9, 90, 3, 	/* w,h,x,y */
	NODATA,
	hang_bluelast,  /* feelme */
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);
static void toggle_stream(Button *b)
{
	toggle_bgroup(b);
	draw_button(&cmu_bluelast_hanger);
}
static Button cmu_stream_sel = MB_INIT1(
	&cmu_bluelast_hanger,
	NOCHILD, 		/* children */
	80, 9, 3, 3, 	/* w,h,x,y */
	NODATA, /* "Stream", */
	ccorner_text,   /* seeme */
	toggle_stream,
	NOOPT,
	&vs.cm_streamdraw, 1,
	NOKEY,
	MB_B_GHILITE /* flags */
	);

Button cmu_pa_group_sel = MB_INIT1(
	NONEXT,
	&cmu_stream_sel,
	80, 9, 0, 0, 	/* w,h,x,y */
	NODATA,		/* datme */
	NOSEE,   /* seeme */
	NOFEEL,  /* feelme */
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
);
/****** move group sel ******/

static Button cmu_moveto_sel = MB_INIT1(
	NONEXT,
	NOCHILD, 		/* children */
	80, 9, 3, 3, 	/* w,h,x,y */
	NODATA, /* "To cursor", */
	ccorner_text,   /* seeme */
	toggle_bgroup,  /* feelme */
	NOOPT,
	&vs.cm_move_to_cursor, 1,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button cmu_move_group_sel = MB_INIT1(
	NONEXT,
	&cmu_moveto_sel,	/* children */
	80, 9, 0, 0, 	/* w,h,x,y */
	NODATA,		/* datme */
	NOSEE,   /* seeme */
	NOFEEL,  /* feelme */
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
);

static Pentool cel_scale_tool = PTOOLINIT1(
	NONEXT,
	NOTEXT, /* "Stretch", */
	PTOOL_OPT,
	CELPT_SCALE,
	"",
	NO_SUBOPTS,
	NULL,
	cel_scale_ptfunc,
	&plain_ptool_cursor,
	init_marqi_ctool, /* on install */
	exit_marqi_ctool /* on remove */
);
static Pentool cel_rotate_tool = PTOOLINIT1(
	&cel_scale_tool,
	NOTEXT, /* "Turn", */
	PTOOL_OPT,
	CELPT_ROTATE,
	"",
	NO_SUBOPTS,
	NULL,
	cel_rotate_ptfunc,
	&plain_ptool_cursor,
	init_marqi_ctool, /* on install */
	exit_marqi_ctool /* on remove */
);
static Pentool cel_move_tool = PTOOLINIT1(
	&cel_rotate_tool,
	NOTEXT, /* "Move", */
	PTOOL_OPT,
	CELPT_MOVE,
	"",
	&cmu_move_group_sel,
	NULL,
	cel_move_ptfunc,
	&plain_ptool_cursor,
	init_marqi_ctool, /* on install */
	exit_marqi_ctool /* on remove */
);
static Pentool cel_paste_tool = PTOOLINIT1(
	&cel_move_tool,
	NOTEXT, /* "Paste", */
	PTOOL_OPT,
	CELPT_PASTE,
	"",
	&cmu_pa_group_sel,
	NULL,
	cel_paste_ptfunc,
	&plain_ptool_cursor,
	init_paste_ctool, /* on install */
	exit_paste_ctool /* on remove */
);
static Pentool cel_paint_tool = PTOOLINIT1(
	&cel_paste_tool,
	NOTEXT, /* "Sprite", */
	PTOOL_OPT,
	CELPT_PAINT,
	"",
	&cmu_pa_group_sel,
	NULL,
	cel_paint_ptfunc,
	&plain_ptool_cursor,
	init_paint_ctool, /* on install */
	exit_paint_ctool /* on remove */
);
static Pentool cel_tcolor_tool = PTOOLINIT1(
	&cel_paint_tool,
	NOTEXT, /* "Set Key", */
	PTOOL_OPT,
	CELPT_TCOLOR,
	"",
	NO_SUBOPTS,
	NULL,
	cel_tcolor_ptfunc,
	&plain_ptool_cursor,
	init_tcolor_ptool, /* on install */
	exit_refresh_cel /* on remove */
);

#define FIRST_CEL_TOOL cel_tcolor_tool

/******* buttons *******/

static Optgroup_data opg = {
	&vs.cur_cel_tool,
	(Option_tool *)&FIRST_CEL_TOOL,
	NULL,
	0
};

static Button cmu_toolopts_sel = MB_INIT1(
	NONEXT, /* next */
	NOCHILD, /* children */
	197, 29, 119, 26, /* w,h,x,y */
	NOTEXT,
	hang_toolopts, 	  /* seeme */
	NOFEEL,
	NOOPT,
	&opg,0,
	NOKEY,
	0 /* flags */
	);

static void see_celtool_sel(Button *b)
{
	set_button_disable(b,thecel == NULL);
	see_option_name(b);
}
static Button cmu_tco_sel = MB_INIT1(
	&cmu_toolopts_sel, /* next */
	NOCHILD, /* children */
	53, 9, 6, 46, /* w,h,x,y */
	&cel_tcolor_tool, /* datme */
	see_celtool_sel,
	mb_set_celtool,
	NOOPT,
	&vs.cur_cel_tool,CELPT_TCOLOR,
	NOKEY,
	MB_GHILITE /* flags */
	);
static void see_draw_frames_option(Button *b)
{
	set_button_disable(b,thecel == NULL 
						 || cmcb->no_draw_tools 
						 || (flix.hdr.frame_count <= 1));
	see_option_name(b);
}
static Button cmu_pai_sel = MB_INIT1(
	&cmu_tco_sel, /* next */
	NOCHILD, /* children */
	53, 9, 6, 26, /* w,h,x,y */
	&cel_paint_tool, 		/* datme */
	see_draw_frames_option,
	mb_set_celtool,
	NOOPT,
	&vs.cur_cel_tool,CELPT_PAINT,
	NOKEY,
	MB_GHILITE /* flags */
	);
static void see_draw_option(Button *b)
{
	set_button_disable(b,cmcb->no_draw_tools);
	see_option_name(b);
}
static Button cmu_pas_sel = MB_INIT1(
	&cmu_pai_sel, /* next */
	NOCHILD, /* children */
	53, 9, 6, 36, /* w,h,x,y */
	&cel_paste_tool, /* datme */
	see_draw_option,
	mb_set_celtool,
	NOOPT,
	&vs.cur_cel_tool,CELPT_PASTE,
	NOKEY,
	MB_GHILITE /* flags */
	);
static Button cmu_sca_sel = MB_INIT1(
	&cmu_pas_sel, /* next */
	NOCHILD, /* children */
	53, 9, 61, 46, /* w,h,x,y */
	&cel_scale_tool, /* datme */
	see_option_name,
	mb_set_celtool,
	NOOPT,
	&vs.cur_cel_tool,CELPT_SCALE,
	NOKEY,
	MB_GHILITE /* flags */
	);
static Button cmu_rot_sel = MB_INIT1(
	&cmu_sca_sel, /* next */
	NOCHILD, /* children */
	53, 9, 61, 36, /* w,h,x,y */
	&cel_rotate_tool, /* datme */
	see_option_name,
	mb_set_celtool,
	NOOPT,
	&vs.cur_cel_tool,CELPT_ROTATE,
	NOKEY,
	MB_GHILITE /* flags */
	);
static Button cmu_mov_sel = MB_INIT1(
	&cmu_rot_sel, /* next */
	NOCHILD, /* children */
	53, 9, 61, 26, /* w,h,x,y */
	&cel_move_tool, /* datme */
	see_option_name,
	mb_set_celtool,
	NOOPT,
	&vs.cur_cel_tool,CELPT_MOVE,
	NOKEY,
	MB_GHILITE /* flags */
	);


static Sgroup1_data cel_sh1dat = {
	&flxtime_data,
};

extern void qgrid_keep_undo(Button *b);

static Button cmu_grid_sel = MB_INIT1(
	NONEXT,
	NOCHILD, /* children */
	44, 9, 267, 14, /* w,h,x,y */
	NODATA, /* "Grid", */
	ncorner_text,
	toggle_bgroup,
	qgrid_keep_undo, 
	&vs.use_grid,1,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
extern void see_mask_button(Button *b);
extern void qmask_keep_undo(Button *b);
static Button cmu_mask_sel = MB_INIT1(
	&cmu_grid_sel, /* next */
	NOCHILD,
	44, 9, 219, 14, /* w,h,x,y */
	NODATA, /* "Mask", */
	see_mask_button,
	mb_toggle_mask,
	qmask_keep_undo,
	&vs.use_mask,1,
	NOKEY,
	MB_B_GHILITE
	);
static Button cmu_zpan_sel = MB_INIT1(
	&cmu_mask_sel,
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
static void see_cel_frames(Button *b)
{
SHORT cnt;
	if(thecel)
		cnt = thecel->flif.hdr.frame_count;
	else
		cnt = 0;
	b->datme = &cnt;
	ncorner_short(b);
}
static Button cmu_frames_sel = MB_INIT1(
	&cmu_zpan_sel,
	NOCHILD, /* children */
	34, 11, 277, 57, /* w,h,x,y */
	NODATA, /* datme */
	see_cel_frames,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	MB_B_GHILITE /* flags */
	);

static Button cmu_minitime_sel =  MB_INIT1(
	&cmu_frames_sel,
	NULL,  		/* children allocated in go_cel_menu() */
	76,11,1,57,
	NODATA, /* "Cel Frames:", */
	see_cel_minitime,
	NOFEEL,
	NOOPT,
	&celtime_data,0,
	NOKEY,
	0
	);
static Button cmu_minipal_sel = MB_INIT1(
	&cmu_minitime_sel, 	/* next */
	&minipal_sel,
	93, 9, 119, 14, /* w,h,x,y */
	NOTEXT,
	hang_children,
	NOFEEL,
	NOOPT,
	NULL,0,
	NOKEY,
	0
	);
static void qcel_inks(Button *b)
{
Pentool *pt;

	/* we can't paste sprite in inks menu !!!! so don't do it */

	pt = vl.ptool;
	if(pt->ot.id == CELPT_PAINT)
	{
		vl.ptool = &null_pentool; /* fudge to prevent going into sprite mode
								   * when in inks menu */
		flx_clear_olays(); /* clear cel if present */
		qinks(b);
		flx_draw_olays(); /* (re)draw cel (if present) */
		vl.ptool = pt;
	}
	else
		qinks();
}
static Button cmu_goinks_sel = MB_INIT1(
	&cmu_minipal_sel,
	NOCHILD,
	53,9,238,3,
	NOTEXT,
	see_cur_ink,
	qcel_inks,
	qcel_inks,
	&vs.ink_id,opq_INKID,
	NOKEY,
	MB_GHILITE
	);
static Button cmu_std1_sel = MB_INIT1(
	&cmu_goinks_sel,
	&std_head1_sel,
	0, 0, 129, 3, /* w,h,x,y */
	NOTEXT,
	hang_children,
	NOFEEL,
	NOOPT,
	&cel_sh1dat,0,
	NOKEY,
	0 /* flags */
	);
Button cmu_common_sels = MB_INIT1(
	&cmu_std1_sel,
	NOCHILD,
	0, 0, 5, 3, /* w,h,x,y */
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);

static Button cmu_undo_sel = MB_INIT1(
	&cmu_mov_sel,
	NOCHILD, /* children */
	33, 9, 93, 3, /* w,h,x,y */
	NODATA, /* "Undo", */
	dcorner_text,
	menu_doundo,
	NOOPT,
	NOGROUP,0,
	'\b',
	0 /* flags */
	);
static Button cmu_title_sel = MB_INIT1(
	&cmu_undo_sel,
	&cmu_common_sels,
	84, 9, 5, 3, 	/* w,h,x,y */
	NODATA, /* "Anim Cel", */
	hang_see_title,
	feel_titlebar,
	mb_menu_to_bottom,
	&tbg_moveclose,0,
	'q',
	0 /* flags */
	);


static void celmenu_credraw(void *dat, USHORT why)
{
	(void)dat;
	(void)why;

	redraw_head1_ccolor(&cmu_std1_sel);
	zpan_ccycle_redraw(&cmu_zpan_sel);
	draw_button(&cmu_minipal_sel);
}
static Redraw_node celmenu_rn = {
	{ NULL, NULL }, /* node */
	celmenu_credraw,
	NULL,
	NEW_CCOLOR };


Menuhdr cel_menu = MENU_INIT0(
	320,70,0,0,  	/* width, height, x, y */
	CEL_MUID,		/* id */
	PANELMENU,		/* type */
	&cmu_title_sel,	/* buttons */
	SCREEN_FONT,	/* font */
	&menu_cursor,	/* cursor */
	seebg_white, 		/* seebg */
	NULL,					/* dodata */
	NULL,					/* domenu */
	(MBPEN|MBRIGHT|KEYHIT), /* ioflags */
	0,				/* flags */
	NULL,			/* procmouse */
	NULL, 			/* on_showhide */
	NULL			/* cleanup */
);

static Smu_button_list cel_smblist[] = {
	{ "title",      { /* butn */ &cmu_title_sel } },
	{ "bluelast",   { /* butn */ &cmu_bluelast_sel } },
	{ "stream",     { /* butn */ &cmu_stream_sel } },
	{ "moveto",     { /* butn */ &cmu_moveto_sel } },
	{ "grid",       { /* butn */ &cmu_grid_sel } },
	{ "mask",       { /* butn */ &cmu_mask_sel } },
	{ "cel_frames", { /* butn */ &cmu_minitime_sel } },
	{ "undo",       { /* butn */ &cmu_undo_sel } },

	/* texts note first char is a 'T' */
	{ "Tscale",     { /* ps */ &cel_scale_tool.ot.name } },
	{ "Tturn",      { /* ps */ &cel_rotate_tool.ot.name } },
	{ "Tmove",      { /* ps */ &cel_move_tool.ot.name } },
	{ "Tpaste",     { /* ps */ &cel_paste_tool.ot.name } },
	{ "Tsprite",    { /* ps */ &cel_paint_tool.ot.name } },
	{ "Tsetkey",    { /* ps */ &cel_tcolor_tool.ot.name } },
};

static void see_cel_minitime(Button *b)
{
	black_label(b);
	mb_hang_chiles_oset(b,b->width, 0);
}
static void redraw_ctool_buttons(void)
{
	draw_button(&cmu_toolopts_sel);
}
static Errcode id_set_ctool(SHORT id)
{
	return(set_curptool((Pentool *)id_find_option(
								   (Option_tool*)&FIRST_CEL_TOOL,id)));
}
static void id_set_cel_tool(SHORT id)
{
	id_set_ctool(id);
	vs.cur_cel_tool = vl.ptool->ot.id;
	redraw_ctool_buttons();
}
static void mb_set_celtool(Button *b)
{
	id_set_ctool(b->identity);
	mb_unhi_group(b);
	vs.cur_cel_tool = vl.ptool->ot.id;
	redraw_ctool_buttons();
	mb_hi_group(b);
}

/***** tool functions ******/

static void exit_refresh_cel(Pentool *pt)
{
	(void)pt;

	if(!thecel || flx_olays_hidden())
		return;
	cmu_unmarqi_cel();
	draw_flicel(thecel,DRAW_DELTA,FORCE_CFIT);
}
static Boolean delta_marqi_cel(void)
{
ULONG time;

	if(!thecel)
		return(1); /* self remove, nothing to marqi */

	if((time = pj_clock_1000()) >= cmcb->marqitime + DMARQI_MILLIS)
	{
		marqi_flicel(thecel,++cmcb->marqmod,NULL); /* re draw with ++mod */
		cmcb->marqitime = time;
	}
	return(0);
}
void cmu_marqi_cel(void)
{
	rem_waitask(&cmcb->mwt); /* just incase it is attached */
	if(thecel)
	{
		init_waitask(&cmcb->mwt,delta_marqi_cel,NULL,0);
		add_waitask(&cmcb->mwt);

		/* just lots of safety for marqiing over existing marqi */
		if(cmcb->marqi_save)
			cmcb->marqi_save = NULL;
		else
			cmcb->marqi_save = cmcb->marqi_save_buf;
		marqi_flicel(thecel,cmcb->marqmod,cmcb->marqi_save);
		cmcb->marqi_save = cmcb->marqi_save_buf;
	}
	else
		cmcb->marqi_save = NULL;
}
void cmu_unmarqi_cel(void)
{
	rem_waitask(&cmcb->mwt); /* stop creepy re-drawing task */
	if(thecel)
		undo_flicel_marqi(thecel,cmcb->marqi_save);
	cmcb->marqi_save = NULL;
}
Errcode init_marqi_ctool(Pentool *pt)
{
	(void)pt;

	if(!flx_olays_hidden())
		cmu_marqi_cel();
	return(Success);
}
void exit_marqi_ctool(Pentool *pt)
{
	(void)pt;
	cmu_unmarqi_cel();
}
/****** scale tool *****/

static Errcode cel_scale_ptfunc(Pentool *pt, Wndo *w)
{
	(void)pt;
	(void)w;

	if(!thecel)
		return Err_bad_input;
	save_celpos_undo();
	cmu_unmarqi_cel();
	vstretch_cel(1);
	cmu_marqi_cel();
	return Success;
}

/**** rotate tool *****/

static Errcode cel_rotate_ptfunc(Pentool *pt, Wndo *w)
{
	(void)pt;
	(void)w;

	if(!thecel)
		return Err_bad_input;
	save_celpos_undo();
	cmu_unmarqi_cel();
	vrotate_cel(1);
	cmu_marqi_cel();
	return Success;
}
/**** move tool *****/

static Errcode cel_move_ptfunc(Pentool *pt, Wndo *w)
{
	(void)pt;
	(void)w;

	if(!thecel)
		return Err_bad_input;
	save_celpos_undo();
	cmu_unmarqi_cel();
	mp_thecel(0,vs.cm_move_to_cursor?2:1);
	cmu_marqi_cel();
	return Success;
}
/***** paint tool *****/

	/*** see celpaste.c ****/

/***** paste tool *****/

	/*** see celpaste.c ****/

/****** tcolor tool *******/

static Errcode draw_solid_tcolor(Pixel dcolor)
{
Errcode err;
BYTE ounder;
BYTE oclear;
Celcfit *ocfit;
Celcfit cfit;
int i;

	/* save old render state */
	ounder = vs.render_under;
	oclear = vs.zero_clear;

	vs.zero_clear = vs.render_under = 0;
	ocfit = thecel->cfit;
	init_celcfit(&cfit);

	if(need_render_cfit(thecel->rc->cmap))
	{
		make_render_cfit(thecel->rc->cmap,&cfit,thecel->cd.tcolor);
	}
	else /* make a xlat for only the tcolor */
	{
		for(i = 0;i < COLORS;++i)
			cfit.ctable[i] = i;
	}
	cfit.ctable[thecel->cd.tcolor] = dcolor;

	thecel->cfit = &cfit;
	err = draw_flicel(thecel,DRAW_FIRST,OLD_CFIT);

	/* restore old render state */
	thecel->cfit = ocfit;
	vs.zero_clear = oclear;
	vs.render_under = ounder;
	return(err);
}
static Errcode init_tcolor_ptool(Pentool *pt)
{
Errcode err;
(void)pt;

	/* if overlays hidden do not draw cel */
	if(!thecel || flx_olays_hidden())
		return(Success);
	if((err = draw_solid_tcolor(vs.inks[1])) >= 0)
		cmu_marqi_cel();
	return(err);
}
static Errcode cel_tcolor_ptfunc(Pentool *pt, Wndo *w)
{
Pixel color;
(void)pt;
(void)w;

	cel_cancel_undo();
	if(!thecel)
		return Err_bad_input;
	if(!isin_fcel(thecel,icb.mx,icb.my))
		return Success; /* TODO - not sure */
	color = celt_color(icb.mx,icb.my);
	set_flicel_tcolor(thecel,color);
	draw_solid_tcolor(vs.inks[1]);
	cmu_marqi_cel();
	return Success;
}
/**************** minitime cel frame control ************************/

static Errcode cm_celdraw(int marqi,int mode)
/* draw or redraw cel for current tool mode */
{
Errcode err;

	if(vs.cur_cel_tool == CELPT_TCOLOR)
		err = draw_solid_tcolor(vs.inks[1]);
	else
		err = draw_flicel(thecel,mode,NEW_CFIT);

	if(err < 0)
		return(err);
	if(marqi)
		cmu_marqi_cel();
	return Success;
}
static Errcode cm_deltadraw(int marqi)
{
	if(thecel)
		return(cm_celdraw(marqi,DRAW_DELTA));
	else
		return Err_nogood;
}
static void mtcel_first_frame(void *flicel)
{
	Flicel **pcel = flicel;
	Flicel *cel;

	if((cel = *pcel) == NULL)
		return;
	seek_fcel_frame(*pcel, 0);
	cm_deltadraw(1);
}
static void mtcel_prev_frame(void *pcel)
{
	Flicel *cel;

	if ((cel = *(Flicel **)pcel) == NULL)
		return;
	seek_fcel_frame(cel, cel->cd.cur_frame - 1);
	cm_deltadraw(1);
}
static void mtcel_playit(void *pcel)
{
Errcode err;
Flicel *cel;
ULONG clock;
Fli_frame *cbuf;

	if ((cel = *(Flicel **)pcel) == NULL)
		return;

	hide_mp();
	hide_mouse();
	cmu_unmarqi_cel();

	if((err = pj_fli_cel_alloc_cbuf(&cbuf,cel->rc)) < 0)
		goto error;

	clock = pj_clock_1000();

	for(;;)
	{
		clock += cel->flif.hdr.speed;
		if (!wait_til(clock))
			break;
		if(cel->flif.hdr.frame_count > 1)
		{
			if((err = gb_seek_fcel_frame(cel, cel->cd.cur_frame + 1,
										 cbuf,FALSE)) < Success)
			{
				goto error;
			}
			if((err = cm_deltadraw(0)) < 0)
				goto error;
		}
	}
	err = Success;
error:
	softerr(err,"cel_play");
	cmu_marqi_cel();
	show_mouse();
	show_mp();
	pj_free(cbuf);
}
static void mtcel_next_frame(void *pcel)
{
	Flicel *cel;

	if ((cel = *(Flicel **)pcel) == NULL)
		return;
	seek_fcel_frame(cel, cel->cd.cur_frame + 1);
	cm_deltadraw(1);
}
static void mtcel_last_frame(void *pcel)
{
	Flicel *cel;

	if ((cel = *(Flicel **)pcel) == NULL)
		return;
	seek_fcel_frame(cel, cel->flif.hdr.frame_count - 1);
	cm_deltadraw(1);
}
static SHORT mtcel_get_framenum(void *pcel)
{
	Flicel *cel;

	if ((cel = *(Flicel **)pcel) == NULL)
		return(0);
	return(cel->cd.cur_frame);
}
static SHORT mtcel_get_framecount(void *pcel)
{
	Flicel *cel;

	if ((cel = *(Flicel **)pcel) == NULL)
		return(1);
	return(cel->flif.hdr.frame_count);
}
static void mtcel_seek_frame(SHORT ix, void *pcel)
{
	Flicel *cel;

	if ((cel = *(Flicel **)pcel) == NULL)
		return;
	seek_fcel_frame(cel, ix);
	cm_deltadraw(1);
}

static Minitime_data celtime_data = {
	mtcel_first_frame, /* first frame */
	mtcel_prev_frame,  /* prev */
	NULL, 			   /* feel ix */
	mtcel_next_frame,  /* next */
	mtcel_playit,	   /* play it */
	mtcel_last_frame,  /* last frame */
	NULL, 			   /* opt_all */
	NULL, 				/* opt tsl_first */
	mtcel_get_framenum, /* get_framenum */
	mtcel_get_framecount, 
	NULL,				/* clear_overlays */
	NULL,				/* draw_overlays */
	mtcel_seek_frame,	/* seek */
	0,					/* overlay clear stack */
	&thecel,            /* data */
};

/* fly playing functions */

void cm_erase_toolcel(void)
{
	if(thecel)
	{
		cmu_unmarqi_cel();
		unsee_flicel(thecel);
	}
}
void cm_restore_toolcel(void)
/* draw cel and its marqi assuming cel is not present on screen */
{
	if(thecel)
		cm_celdraw(1,DRAW_FIRST);
}

static void ring_seek_fli(int ix)
{
int new_ix;

	if(ix == (vs.frame_ix + 1))
	{
		new_ix = flx_ringseek(vb.pencel, vs.frame_ix, ix);
		zoom_it();
		save_undo();
	}
	else
	{
		save_undo();
		new_ix = flx_ringseek(undof, vs.frame_ix, ix);
		zoom_unundo();
	}
	if(new_ix >= 0)
		vs.frame_ix = new_ix;
}

static void ring_seek_fli_with_data(SHORT ix, void *data)
{
	(void)data;
	ring_seek_fli(ix);
}

static void cm_firstfli(void *data)
{
	(void)data;

	if(cmcb->num_overlays)
		ring_seek_fli(0);
	else
		first_frame();
} 
static void cm_nextfli(void *data)
{
	(void)data;

	if(cmcb->num_overlays)
		ring_seek_fli(vs.frame_ix + 1);
	else
		next_frame();
} 
static void cm_prevfli(void *data)
{
	(void)data;

	if(cmcb->num_overlays)
		ring_seek_fli(vs.frame_ix - 1);
	else
		prev_frame();
} 

extern SHORT flx_get_framecount(), flx_get_frameix();
extern void go_time_menu(), mplayit(), last_frame();

static char cel_off = 0; /* this brackets scrubs and flx seeks */
static void cm_clear_olays(void *data)
{
	(void)data;

	if(cel_off == 0)
	{
		cmu_free_paste_undo(); /* kill paste undo and free mem */
		cm_erase_toolcel();
	}
	++cel_off;
}
static void cm_draw_olays(void *data)
{
	(void)data;

	if((--cel_off) == 0)
		cm_restore_toolcel();
}
static Minitime_data cm_flitime_data = {
	cm_firstfli,  /* first frame */
	cm_prevfli,   /* prev */
	go_time_menu, /* ix */
	cm_nextfli,   /* next */
	mplayit,	  /* play it */
	last_frame,   /* last frame */
	go_time_menu,	/* opt_all */
	NULL, 			/* opt tsl_first */
	flx_get_frameix, /* get frame num */
	flx_get_framecount,	/* get framecount */
	cm_clear_olays,    /* clear_overlays */
	cm_draw_olays,  /* draw_overlays */
	ring_seek_fli_with_data, /* seek */
	0,            /* olay_stack */
	NULL, /* data */
};

/********************************************/
static void cleanup_cel_menu(void)
{
	free_buttonlist(&(cmu_minitime_sel.children));
	if(thecel)
		close_fcelio(thecel);

	set_curptool(NULL); /* clean up tool with its exit function */
	cmu_unmarqi_cel();  /* this is needed to insure the input creepy task
						 * is removed */

	if(cmcb->undo_corrupted)
	{
		cm_erase_toolcel(); /* erase cel, then restore contents of undo */
		save_undo();
	}
	else
		zoom_unundo(); /* make sure any bits of stuff are restored */

	pj_freez(&cmcb->marqi_save_buf); /* free old marqi buffer */
}

Errcode reset_celmenu(int toolid,Boolean startup)

/* called if cel changes with pen tool id to be loaded assumes cel is absent
 * from screen and current pen tool is not a cel tool */
{
Errcode err;

	/* on startup we save undo first since the cel is not up and we dont want 
	 * the unsee cel to undo that section of the screen otherwise we do want 
	 * to erase the cel from the undo area and the undo area is valid */

	if(startup)
		save_undo();
	flx_clear_olays(); /* clear cel if present or just decrement stack */
	if(!startup)
		save_undo();
	cmcb->undo_corrupted = 0;
	free_flx_overlays(&flix);

	if(!thecel)
	{
		if(toolid == CELPT_TCOLOR)
			toolid = CELPT_MOVE;
	}
	if(cmcb->no_draw_tools || !thecel)
	{
		if(toolid == CELPT_PASTE 
			|| toolid == CELPT_PAINT
			|| toolid == NULL_PTOOL)
		{
			toolid = CELPT_MOVE;
		}
	}
	if(thecel)
	{
		if((err = reopen_fcelio(thecel, JREADONLY)) < 0)
			goto error;
	}
	id_set_cel_tool(toolid);
	err = Success;
error:
	flx_draw_olays(); /* (re)draw cel (if present) using overlay stack pop */
	return(softerr(err,"cel_install"));
}

static void toolcel_color_redraw(void *dat, USHORT why)
{
	(void)dat;

	if(flx_olays_hidden()
		|| flxtime_data.draw_overlays != cm_draw_olays) 
	{
		return;
	}

	if(    ((why & NEW_CCOLOR) && vs.render_one_color)
		|| ((why & NEW_INK0) && vs.render_under)
		|| ((why & NEW_INK1) && vs.cur_cel_tool == CELPT_TCOLOR)
		|| ((why & NEW_CMAP) && vs.fit_colors)
		|| ((why & NEW_CEL_TCOLOR) && vs.zero_clear))
	{
		cm_deltadraw(1); /* this is a delta because render is different */
	}
}

static void toolcel_rmode_redraw(void *dat, USHORT why)
{
	(void)dat;
	(void)why;

	cm_deltadraw(1);
}

static Redraw_node toolcel_rn = {
	{ NULL, NULL }, /* node */
	toolcel_color_redraw,
	NULL,
	NEW_CCOLOR|NEW_INK0|NEW_INK1|NEW_CMAP|NEW_CEL_TCOLOR };

static Redraw_node tcel_rmode_rn = {
	{ NULL, NULL }, /* node */
	toolcel_rmode_redraw,
	NULL,
	(RSTAT_ZCLEAR|RSTAT_CFIT|RSTAT_UNDER|RSTAT_ONECOL) };

void enable_toolcel_redraw(void)
{
	add_color_redraw(&toolcel_rn);
	add_rmode_redraw(&tcel_rmode_rn);
}
void disable_toolcel_redraw(void)
{
	rem_color_redraw(&toolcel_rn);
	rem_rmode_redraw(&tcel_rmode_rn);
}

static Boolean do_celmenu_keys(void)
{
 	if(check_toggle_abort()
		|| common_header_keys()
		|| check_undo_key())
	{
		return(TRUE);
	}
	return FALSE;
}

static void do_cel_menu(Boolean no_draw_tools)
{
Errcode err;
Celmu_cb celcb;
Minitime_data oflxdata;
Pentool *optool;
void *oredo;
void *oundo;
void *softbs = NULL;
static char panel_key[] = "cel_panel";


	if(MENU_ISOPEN(&cel_menu))
		return;

	clear_struct(&celcb); 
	/* clear, and plug stack buffer into global pointer */
	cmcb = &celcb;

	flx_clear_olays(); /* clear whatever might be there tween ? */
	oflxdata = flxtime_data; /* save global one */

	oredo = vl.redoit;
	oundo = vl.undoit;
	vl.redoit = NULL;
	vl.undoit = NULL;

	flxtime_data = cm_flitime_data; /* and install this one */ 

	hide_mp();
	celcb.no_draw_tools = no_draw_tools;
	optool = vl.ptool;
	fliborder_on(); /* restore border on fli window */

	if(( err = soft_buttons(panel_key, cel_smblist, Array_els(cel_smblist),
							&softbs )) < Success)
	{
		goto error;
	}

	if ((err = load_soft_pull(&cmcb->tpull, 12, "cel", CELPULL_MUID,
		cm_selit, do_celpull)) < Success)
		goto error;

	/* we need two copies of the minitime buttons, one for the cel and one for 
	 * the fli */

	if((err = clone_buttonlist(&timeslider_sel,
							&cmu_minitime_sel.children)) < Success)
	{
		goto error;
	}
	
	/* allocate marqi_save buffer used to save area behind cel's marqi */

	if((err = ealloc(&cmcb->marqi_save_buf,
		((vb.pencel->width * vb.pencel->height)+4) * sizeof(Pixel))) < Success)
	{
		goto error;
	}

	if((err = reset_celmenu(vs.cur_cel_tool,TRUE)) < Success)
		goto error;

	enable_toolcel_redraw(); /* enable redraw functions for render state 
							  * change broadcasts */
	add_color_redraw(&celmenu_rn);  /* for refreshing menu for ccolor */
	menu_to_quickcent(&cel_menu);
	err = do_menuloop(vb.screen,&cel_menu,NULL,
					  &cmcb->tpull,do_celmenu_keys);
	rem_color_redraw(&celmenu_rn);  /* turn off ccolor refreshing */
	disable_toolcel_redraw(); /* no more cel redrawing */
error:
	cleanup_cel_menu();
	softerr(err,"cel_menu");
	vl.redoit = oredo; /* restore old undo and redo functions */
	vl.undoit = oundo;
	restore_pentool(optool);
	smu_free_scatters(&softbs);
	smu_free_pull(&cmcb->tpull);	/* free up loaded pull-downs */
	cmcb = NULL;	 /* make sure no-one uses this above us (paranoia) */
	flxtime_data = oflxdata; /* restore old fli seek control block */
	flx_draw_olays(); /* restore what was there */
	show_mp();
}
void go_cel_menu(void)
{
	do_cel_menu(0);
}
void go_nodraw_cel_menu(void)
{
	do_cel_menu(1);
}
