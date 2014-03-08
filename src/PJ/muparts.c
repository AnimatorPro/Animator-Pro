#define MUPARTS_INTERNALS
#include "jimk.h"
#include "broadcas.h"
#include "flicel.h"
#include "menus.h"
#include "softmenu.h"

extern void ccolor_box(), see_pen(), ppalette(), set_pbrush();
extern void toggle_zoom(), toggle_pen(), go_cel_menu(), go_multi();
void shortcut_ccycle(Button *b);
void go_color_grid(Button *b);
extern Button tseg_a_sel, tseg_s_sel, tseg_f_sel;

/**** standard header group 1 the right side of the home menu ****/

#define SHG ((Sgroup1_data *)(b->group))

static void sh1_hang_minitime(Button *b)
{
Sgroup1_data *shg;

	shg = b->group;
	b->group = shg->minidat;
	hang_children(b);
	b->group = shg;
}
#undef SHG

Button sh1_brush_sel = MB_INIT1(
	NONEXT,
	NOCHILD, /* children */
	12, 12, 163, -2, /* w,h,x,y */
	NOTEXT,	/* datme */
	see_pen,
	toggle_pen,
	set_pbrush,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);
static Button sh1_tco_sel = MB_INIT1(
	&sh1_brush_sel, /* next */
	NOCHILD, /* children */
	11, 9, 96, 0, /* w,h,x,y */
	NODATA, /* "K", */
	ncorner_text,
	mb_toggle_zclear,
	go_cel_menu,
	&vs.zero_clear, 1,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button sh1_filp_sel = MB_INIT1(
	&sh1_tco_sel, /* next */
	NOCHILD, /* children */
	11, 9, 86, 0, /* w,h,x,y */
	NODATA, /* "F", */
	ncorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.fillp, 1,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button sh1_tim_sel = MB_INIT1(
	&sh1_filp_sel, /* next */
	NOCHILD, /* children */
	11, 9, 76, 0, /* w,h,x,y */
	NODATA, /* "T", */
	ncorner_text,
	toggle_bgroup,
	go_multi,
	&vs.multi, 1,
	NOKEY,
	MB_B_GHILITE /* flags */
	);
static Button sh1_ccolor_sel = MB_INIT1(
	&sh1_tim_sel, /* next */
	NOCHILD, /* children */
	11, 11, 177, -1, /* w,h,x,y */
	NOTEXT, 		/* datme */
	ccolor_box,
	go_color_grid,
	ppalette,
	NOGROUP,-0xAA,
	NOKEY,
	0 /* flags */
	);
Button std_head1_sel = MB_INIT1(
	&sh1_ccolor_sel, /* next */
	&minitime_sel, /* children */
	76, 9, 0, 0, /* w,h,x,y */
	NOTEXT, 	 /* datme */
	sh1_hang_minitime,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	MB_GHANG /* flags */
	);

void redraw_head1_ccolor(Button *hanger)

/* redraws parts of a std_head1_sel dependent on ccolor from hanger sel */
{
Button *b;

	if(!hanger || (b = hanger->children) == NULL)
		return;
	draw_buttontop(b->next);
}

/****** mini palette sel ******/

static void see_inkwell(Button *b)
{
SHORT color;

	color = vs.inks[b->identity];
	mb_dinside(b,color);
	if (vs.inks[b->identity] == vs.ccolor)
		color = mc_red(b);
	else
		color = mc_grey(b);
	mb_dcorner(b,color);
}



void pget_color(), fill_inkwell();


static Button mp_ink7_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	12, 9, 80, 0, /* w,h,x,y */
	NULL,
	see_inkwell,
	pget_color,
	fill_inkwell,
	NOGROUP,7,
	NOKEY,
	MB_SCALE_ABSW
	);
static Button mp_ink6_sel = MB_INIT1(
	&mp_ink7_sel,
	NOCHILD,
	12, 9, 69, 0, /* w,h,x,y */
	NULL,
	see_inkwell,
	pget_color,
	fill_inkwell,
	NOGROUP,6,
	NOKEY,
	MB_SCALE_ABSW
	);
static Button mp_ink5_sel = MB_INIT1(
	&mp_ink6_sel,
	NOCHILD,
	12, 9, 58, 0, /* w,h,x,y */
	NULL,
	see_inkwell,
	pget_color,
	fill_inkwell,
	NOGROUP,5,
	NOKEY,
	MB_SCALE_ABSW
	);
static Button mp_ink4_sel = MB_INIT1(
	&mp_ink5_sel,
	NOCHILD,
	12, 9, 47, 0, /* w,h,x,y */
	NULL,
	see_inkwell,
	pget_color,
	fill_inkwell,
	NOGROUP,4,
	NOKEY,
	MB_SCALE_ABSW
	);
static Button mp_ink3_sel = MB_INIT1(
	&mp_ink4_sel,
	NOCHILD,
	12, 9, 36, 0, /* w,h,x,y */
	NULL,
	see_inkwell,
	pget_color,
	fill_inkwell,
	NOGROUP,3,
	NOKEY,
	MB_SCALE_ABSW
	);
static Button mp_ink2_sel = MB_INIT1(
	&mp_ink3_sel,
	NOCHILD,
	12, 9, 25, 0, /* w,h,x,y */
	NULL,
	see_inkwell,
	pget_color,
	fill_inkwell,
	NOGROUP,2,
	NOKEY,
	MB_SCALE_ABSW
	);
static Button mp_ink1_sel = MB_INIT1(
	&mp_ink2_sel,
	NOCHILD,
	12, 9, 14, 0, /* w,h,x,y */
	NULL,
	see_inkwell,
	pget_color,
	fill_inkwell,
	NOGROUP,1,
	NOKEY,
	MB_SCALE_ABSW
	);
static Button mp_ink0_sel = MB_INIT1(
	&mp_ink1_sel,
	NOCHILD,
	12, 9, 0, 0, /* w,h,x,y */
	NULL,
	see_inkwell,
	pget_color,
	fill_inkwell,
	NOGROUP,0,
	NOKEY,
	0
	);
Button minipal_sel = MB_INIT1(
	NONEXT,
	&mp_ink0_sel,
	81, 9, 0, 0, /* w,h,x,y */
	NULL,
	NOSEE,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);

void mb_toggle_zclear(Button *b)
{
	toggle_bgroup(b);
	do_rmode_redraw(RSTAT_ZCLEAR);
}
static void pget_color(Button *b)

/* this called from within a feelme so mx my are good */
{
	if (    icb.mx > b->x 
		 && icb.mx < (b->x + b->width - 1) 
		 && icb.my > b->y 
		 && icb.my < (b->y + b->height - 1))
	{
		vs.cycle_draw = 0;
		update_ccolor(pj_get_dot(b->root,icb.mx, icb.my));
	}
}
static void show_ink(int c)
{
Rgb3 m;
static int lastc;

	if(c >= 0 && c != lastc)
	{
		get_color_rgb(c,vb.pencel->cmap,&m);
		soft_top_textf("!%-3d%-3d%-3d%-3d", "top_color", c, m.r, m.g, m.b);
		draw_buttontop(&sh1_ccolor_sel);
	}
	lastc = c;
}
static void fill_inkwell(Button *b)
{
int c;
Cursorhdr *ocurs;
USHORT changes = 0;
SHORT occolor = vs.ccolor;

	ocurs = set_pen_cursor(&pick_cursor);
	mb_dcorner(b,mc_red(b));
	show_ink(-1);
	if((c = get_a_end(show_ink))>=0)
	{
		changes |= NEW_MINIPAL_INK;

		if(b->identity == 0)
		{
			if(thecel && (thecel->cd.tcolor != c))
			{
				set_flicel_tcolor(thecel,c);
				changes |= NEW_CEL_TCOLOR;
			}
			if(vs.inks[0] != c)
				changes |= NEW_INK0;
		}
		if(b->identity == 1 && vs.inks[1] != c)
			changes |= NEW_INK1;

#ifdef SET_CCOLOR
		if(c != occolor)
		{
			set_ccolor(c);
			changes |= NEW_CCOLOR;
		}
#endif /* SET_CCOLOR */

		vs.inks[b->identity] = c;
	}
	set_pen_cursor(ocurs);

#ifndef SET_CCOLOR
	draw_buttontop(&sh1_ccolor_sel);
#endif /* SET_CCOLOR */

	do_color_redraw(changes);
	see_inkwell(b);
}

/****** zoom cycle_color pan group *********/

extern void movefli_tool(), go_zoom_settings();

extern void mb_toggle_ccycle();

static void see_ccycle(Button *b)
{
	mb_set_hilite(b,vs.cycle_draw);
	ncorner_text(b);
}
static Button zpg_pan_sel = MB_INIT1(
	NONEXT,
	NOCHILD, /* children */
	38, 9, 168, 14, /* w,h,x,y */
	NODATA, /* "Pan", */
	ncorner_text,
	movefli_tool,
	movefli_tool,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);
static Button zpg_cycle_sel = MB_INIT1(
	&zpg_pan_sel, /* next */
	NOCHILD, /* children */
	11, 9, 155, 14, /* w,h,x,y */
	NODATA, /* "C", */
	see_ccycle,
	mb_toggle_ccycle,
	shortcut_ccycle,
	NOGROUP,0,
	NOKEY,
	MB_GHANG /* flags */
	);
static see_zoom(Button *b)
{
	set_button_disable(b,zoom_disabled());
	ncorner_text(b);
}
Button zpan_cycle_group = MB_INIT1(
	&zpg_cycle_sel, /* next */
	NOCHILD,
	39, 9, 114, 14, /* w,h,x,y */
	NODATA, /* "Zoom", */
	see_zoom,
	toggle_zoom,
	go_zoom_settings,
	&vs.zoom_open,1,
	'z',
	MB_B_GHILITE
	);
void zpan_ccycle_redraw(Button *hanger)
{
	if(hanger && hanger->children)
		draw_button(hanger->children->next);
}

/* texts used in vmarqi.c */

char *rub_line_str; 
	/* "(![1] ![2]) wid ![3] hgt ![4] (![5] ![6]) deg ![7] rad ![8]" */

char *box_coor_str; /* "(![1] ![2] ) (![3] ![4] )" */
char *rub_rect_str; /* "![1] ![2]  (![3] ![4])  ![5] ![6]" */
char *rub_circle_str; /* "R = ![1] D = ![2]" */

static Smu_button_list mup_smblist[] = {
	{ "kcol", &sh1_tco_sel },
	{ "fillp", &sh1_filp_sel },
	{ "otime", &sh1_tim_sel },
	{ "pan", &zpg_pan_sel },
	{ "ccycle", &zpg_cycle_sel },
	{ "zoom", &zpan_cycle_group },
	{ "tseg_all", &tseg_a_sel },   /* button in tseg.c */
	{ "tseg_seg", &tseg_s_sel },   /* button in tseg.c */
	{ "tseg_frame", &tseg_f_sel }, /* button in tseg.c */
	{ "pbrush", &sh1_brush_sel },

	// texts start with 'T'

	{ "Trub_line", &rub_line_str }, 
	{ "Tbox_coor", &box_coor_str },
	{ "Trub_rect", &rub_rect_str },
	{ "Trub_circle", &rub_circle_str },
};

static void *mupss;
Errcode init_menu_parts()
{
	return(soft_buttons("pj_muparts",mup_smblist, 
			Array_els(mup_smblist),&mupss));
}
void cleanup_menu_parts()
{
	smu_free_scatters(&mupss);
}
