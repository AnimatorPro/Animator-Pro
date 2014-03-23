/* palet2.c - The main bits of code for the palette editor.  Also a lot
   of related code in cluster.c and palpull.c.  This file starts with
   the Button button declarations for the palette editor control panel. */

#include "jimk.h"
#include "aaconfig.h"
#include "auto.h"
#include "broadcas.h"
#include "brush.h"
#include "commonst.h"
#include "errcodes.h"
#include "flicel.h"
#include "inks.h"
#include "memory.h"
#include "menus.h"
#include "palchunk.h"
#include "palmenu.h"
#include "palpul.h"
#include "rastcurs.h"
#include "softmenu.h"

extern Button inks_sel, ccolor_sel, spec1_sel;
extern void go_multi(),
	see_undo(), toggle_pen();

void go_color_grid(Button *b);

extern void swap_undo();

static void pal_feel_qslider(Button *m);
static void sliders_from_ccolor(void);
static void see_color_sliders(void);
static void ccolor_from_sliders(void);
static void change_hls_mode(Button *m);
static void pal_menu_back(Button *m);

static SHORT ccred,ccgreen,ccblue;
#ifdef SLUFFED
static UBYTE *cbuf;
#endif /* SLUFFED */
static Cmap *new_cmap;

static Qslider red_sl = 
	QSL_INIT1(0,RGB_MAX-1,&ccred,0,ccolor_from_sliders,leftright_arrs);
static Qslider green_sl = 
	QSL_INIT1(0,RGB_MAX-1,&ccgreen,0,ccolor_from_sliders,leftright_arrs);
static Qslider blue_sl = 
	QSL_INIT1(0,RGB_MAX-1,&ccblue,0,ccolor_from_sliders,leftright_arrs);


/*** Button Data ***/


Button pal_pal_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	320,58,0,(142)-103,
	NOTEXT,
	see_powell_palette,
	feel_pp,
	right_click_pp,
	NULL,0,
	NOKEY,
	MB_SCALE_ABS
	);
static Button pal_bsl_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	86+1,10+1,229,(129)-103,
	&blue_sl,
	see_qslider,
	pal_feel_qslider,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button pal_gsl_sel = MB_INIT1(
	&pal_bsl_sel,
	NOCHILD,
	85+1,10+1,141,(129)-103,
	&green_sl,
	see_qslider,
	pal_feel_qslider,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button pal_rsl_sel = MB_INIT1(
	&pal_gsl_sel,
	NOCHILD,
	85+1,10+1,53,(129)-103,
	&red_sl,
	see_qslider,
	pal_feel_qslider,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button pal_hls_sel = MB_INIT1(
	&pal_rsl_sel,
	NOCHILD,
	22+1,10+1,28,(129)-103,
	NODATA,		/* Read in from menu file */
	dcorner_text,
	change_hls_mode,
	NOOPT,
	&vs.hls,1,
	NOKEY,
	MB_B_GHILITE
	);
static Button pal_rgb_sel = MB_INIT1(
	&pal_hls_sel,
	NOCHILD,
	22+1,10+1,3,(129)-103,
	NODATA,		/* Read in from menu file */
	dcorner_text,
	change_hls_mode,
	NOOPT,
	&vs.hls,0,
	NOKEY,
	MB_B_GHILITE
	);
Button pal_cco_sel = MB_INIT1(
	&pal_rgb_sel,
	NOCHILD,
	11,11,306,105-103,
	NOTEXT,
	ccolor_box,
	go_color_grid,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
Button pal_bru_sel = MB_INIT1(
	&pal_cco_sel,
	NOCHILD,
	11,11,293,105-103,
	NOTEXT,
	see_pen,
	toggle_pen,
	set_pbrush,
	NULL,0,
	NOKEY,
	0
	);
Button pal_spe_sel = MB_INIT1(
	&pal_bru_sel,
	NOCHILD,
	85,10,232,117-103,
	NOTEXT,
	see_cluster,
	feel_cluster,
	mselect_bundle,
	NULL,1,
	NOKEY,
	0
	);
static Button pal_tsp_sel = MB_INIT1(
	&pal_spe_sel,
	NOCHILD,
	15,9,215,117-103,
	NODATA,		/* Read in from menu file */
	dcorner_text,
	change_cluster_mode,
	NOOPT,
	&vs.use_bun,1,
	NOKEY,
	MB_B_GHILITE	
	);
Button pal_bun_sel = MB_INIT1(
	&pal_tsp_sel,
	NOCHILD,
	85,10,126,(117)-103,
	NOTEXT,
	see_cluster,
	feel_cluster,
	mselect_bundle,
	NULL,0,
	NOKEY,
	0
	);
static Button pal_tbu_sel = MB_INIT1(
	&pal_bun_sel,
	NOCHILD,
	15,9,109,117-103,
	NODATA,		/* Read in from menu file */
	dcorner_text,
	change_cluster_mode,
	NOOPT,
	&vs.use_bun,0,
	NOKEY,
	MB_B_GHILITE	
	);
static Button pal_clu_sel = MB_INIT1(
	&pal_tbu_sel,
	NOCHILD,
	47,9,246,106-103,
	NODATA,		/* Read in from menu file */
	dcorner_text,
	change_mode,
	NOOPT,
	&vs.pal_to,0,
	NOKEY,
	MB_GHILITE
	);
static Button pal_all_sel = MB_INIT1(
	&pal_clu_sel,
	NOCHILD,
	25,9,219,(106)-103,
	NODATA,		/* Read in from menu file */
	dcorner_text,
	change_mode,
	NOOPT,
	&vs.pal_to,1,
	NOKEY,
	MB_GHILITE
	);
static Button pal_minipal_sel = MB_INIT1(
	&pal_all_sel,
	&minipal_sel,
	92,9,8,117-103,
	NOTEXT,
	hang_children,
	NOFEEL,
	NOOPT,
	NULL,0,
	NOKEY,
	0
	);
static Button pal_m_sel = MB_INIT1(
	&pal_minipal_sel,
	NOCHILD,
	11,9,205,106-103,
	NODATA,		/* Read in from menu file */
	ncorner_text,
	toggle_bgroup,
	go_multi,
	&vs.multi,1,
	NOKEY,
	MB_B_GHILITE
	);
extern Minitime_data flxtime_data;

static Button pal_min_sel = MB_INIT1(
	&pal_m_sel,
	&minitime_sel,
	77,9,129,(106)-103,
	NOTEXT,
	hang_children,
	NOFEEL,
	NOOPT,
	&flxtime_data,0,
	NOKEY,
	0
	);
static Button pal_fit_sel = MB_INIT1(
	&pal_min_sel,
	NOCHILD,
	33,9,94,106-103,
	NODATA,		/* Read in from menu file */
	ncorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.pal_fit,1,
	NOKEY,
	MB_B_GHILITE
	);
static Button pal_res_sel = MB_INIT1(
	&pal_fit_sel,
	NOCHILD,
	33,9,59,106-103,
	NODATA,		/* Read in from menu file */
	see_undo,
	menu_doundo,
	NOOPT,
	NOGROUP,0,
	'\b',
	0
	);
static Button pal_tit_sel = MB_INIT1(
	&pal_res_sel,
	NOCHILD,
	54,9,3,(106)-103,
	NODATA,		/* Read in from menu file */
	see_titlebar,
	feel_titlebar,
	mb_menu_to_bottom,
	&tbg_moveclose,0,
	'q',
	0 /* flags */
	);
static Button pal_most_sel = MB_INIT1(
	&pal_pal_sel,
	&pal_tit_sel,
	320,40,0,0,
	NOTEXT,
	pal_menu_back,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	MB_SCALE_ABS
	);

static Smu_button_list pal_blist[] = {
	{ "title",  { &pal_tit_sel } },
	{ "hls",    { &pal_hls_sel } },
	{ "rgb",    { &pal_rgb_sel } },
	{ "b",      { &pal_tsp_sel } },
	{ "a",      { &pal_tbu_sel } },
	{ "clus",   { &pal_clu_sel } },
	{ "all",    { &pal_all_sel } },
	{ "t",      { &pal_m_sel } },
	{ "fit",    { &pal_fit_sel } },
	{ "undo",   { &pal_res_sel } },
	{ "pen",    { &pal_bru_sel } },
	};

static void pmu_color_redraw(Menuhdr *mh, USHORT why)
{
	(void)mh;
	(void)why;

	draw_buttontop(&pal_bun_sel);
	draw_buttontop(&pal_cco_sel);
	draw_button(&pal_minipal_sel);
	draw_buttontop(&pal_spe_sel);
	draw_buttontop(&pal_pal_sel);
	see_color_sliders();
}
static Redraw_node palette_rn = {
	{ NULL, NULL }, /* node */
	pmu_color_redraw,
	NULL,
	NEW_CCOLOR|NEW_CMAP };

static void pmu_on_showhide(Menuhdr *mh,Boolean showing)
{
	(void)mh;

	if(showing)
		add_color_redraw(&palette_rn);
	else
		rem_color_redraw(&palette_rn);
}
Menuhdr palette_menu = MENU_INIT0(
	320,97,0,103,		/* width, height, x, y */
	PALETTE_MUID,   	/* id */
	PANELMENU,			/* type */
	&pal_most_sel,		/* buttons */
	SCREEN_FONT, 		/* font */
	&menu_cursor.hdr,	/* cursor */
	sliders_from_ccolor,	/* seebg */
	NULL,					/* dodata */
	NULL,					/* domenu */
	(MBPEN|MBRIGHT|KEYHIT), /* ioflags */
	0,				/* flags */
	NULL,			/* procmouse */
	pmu_on_showhide, /* on_showhide */
	NULL			/* cleanup */
);

static void pal_feel_qslider(Button *m)
{
	save_undo();
	feel_qslider(m);
	dirties();
#ifdef NOT_NEEDED
	do_color_redraw(NEW_CMAP);
#endif
}

static Errcode
cmapcopy1(void *data, int ix, int intween, int scale, Autoarg *aa)
{
	(void)data;
	(void)ix;
	(void)intween;
	(void)scale;
	(void)aa;

	pj_cmap_copy(new_cmap, vb.pencel->cmap);
	return Success;
}

static Errcode refit1(void *data, int ix, int intween, int scale, Autoarg *aa)
{
	if (vs.pal_fit)
		refit_rcel(vb.pencel, new_cmap, vb.pencel->cmap);
	return cmapcopy1(data, ix, intween, scale, aa);
}
Errcode load_palette(char *title, int fitting)

/* loads a palette from a palette file or a fli reports errors */
{
Errcode err;
Errcode fliret;
Flifile flif;
autoarg_func how;

	if(title == NULL)
		return(Err_bad_input);

	if ((err = pj_cmap_alloc(&new_cmap,COLORS)) < Success)
		goto error;

	if((err = pj_col_load(title, new_cmap)) < Success)
	{
		if(pj_fli_open(title,&flif,JREADONLY) < Success)
			goto error;

		if((fliret = fli_read_colors(&flif,new_cmap)) < Success)
		{
			if(fliret == Err_no_record)
			{
				err = Err_reported;
				soft_continu_box("!%s","no_palette", title);
			}
			goto error;
		}
	}

	swap_cmaps(vb.pencel->cmap, new_cmap);
	see_cmap();
	if (fitting == 1)
		fitting = soft_yes_no_box("color_fit");
	if (fitting)
		how = refit1;
	else
		how = cmapcopy1;
	/* for undo to work change colors again...*/
	swap_cmaps(vb.pencel->cmap, new_cmap);
	see_cmap();
	err = uzauto(how, NULL);
error:
	pj_cmap_free(new_cmap);
	pj_fli_close(&flif);
	return(cant_load(err,title));
}

void qload_palette(void)
{
char *title;
char buf[50];

	if ((title =  vset_get_filename(stack_string("load_pal",buf),
								".COL;.FLC;.CEL", load_str,
							    PALETTE_PATH, NULL, 0)) != NULL)
	{
		load_palette(title, 1);
	}
}


void qsave_palette(void)
{
char *title;
char buf[50];

	title =  vset_get_filename(stack_string("save_pal",buf),
							".COL", save_str,
							  	PALETTE_PATH,NULL,1);
	if (title != NULL)
	{
		if (overwrite_old(title))
		{
			softerr(pj_col_save(title,vb.pencel->cmap), 
							"!%s", "cant_save", title );
		}
	}
}

static void refit_1c(int scale, Rgb3 *p, int ccolor, int ix)
{
	(void)scale;
	(void)ix;

	get_color_rgb(ccolor,new_cmap,p);
}



void refit_vf(void)
{
	if (vs.pal_fit)
		refit_rcel(vb.pencel, vb.pencel->cmap, undof->cmap);
}

static Errcode
cl_refit1(void *data, int ix, int intween, int scale, Autoarg *aa)
{
	(void)data;
	(void)ix;
	(void)intween;
	(void)scale;
	(void)aa;

	some_cmod(refit_1c, SCALE_ONE);
	return Success;
}

void cdefault(void)
{
extern Cmap *pj_default_cmap;

	new_cmap = pj_default_cmap;
	pmhmpauto(cl_refit1, NULL);
}

void cuse_cel(void)
{
	if (thecel != NULL)
	{
		if ((new_cmap = clone_cmap(thecel->rc->cmap)) != NULL)
		{
			pmhmpauto(cl_refit1, NULL);
			pj_cmap_free(new_cmap);
		}
	}
}



static void sliders_from_ccolor(void)
{
Rgb3 *rgb;
SHORT r, g, b;

	rgb = vb.pencel->cmap->ctab + vs.ccolor;
	r = rgb->r;
	g = rgb->g;
	b = rgb->b;
	if (vs.hls)
	{
		rgb_to_hls(r, g, b, &ccred, &ccgreen, &ccblue);
		red_sl.max = green_sl.max = blue_sl.max = 255;
	}
	else
	{
		ccred = r;
		ccgreen = g;
		ccblue = b;
		red_sl.max = green_sl.max = blue_sl.max = RGB_MAX-1;
	}
}
static void see_color_sliders(void)
{
Button **s, *f;
int i;
static Button *slides[3] = {&pal_rsl_sel, &pal_gsl_sel, &pal_bsl_sel};

	sliders_from_ccolor();
	s = slides;
	i = 3;
	while (--i >= 0)
	{
		f = *s++;
		white_block(f);
		draw_buttontop(f);
	}
}

void rampit(UBYTE *r1, UBYTE *r2, UBYTE *dr, int ccount)
/* Make a color smooth RGB color ramp between r1 and r2 into dr. */
{
int i,j;

for (i=0; i<ccount; i++)
	{
	for (j=0; j<3; j++)
		dr[j] = interp_range(r1[j], r2[j],  i, ccount);
	dr += 3;
	}
}

void hls_rampit(Rgb3 *r1, Rgb3 *r2, Rgb3 *dr, int ccount)
/* Make a color smooth RGB color ramp between r1 and r2 into dr. */
{
	SHORT h1,l1,s1;
	SHORT h2,l2,s2;
	SHORT r,g,b,h,l,s;
	int dh, dl, ds;
	int diver = ccount-1;
	int rounder = diver/2;
	int i;

	/* Convert end points from rgb to hls coordinates. */
	rgb_to_hls(r1->r, r1->g, r1->b, &h1, &l1, &s1);
	rgb_to_hls(r2->r, r2->g, r2->b, &h2, &l2, &s2);
	dh = h2-h1;
	if (dh <= 0)
		dh += 256.0;
	dl = l2-l1;
	ds = s2-s1;
	for (i=0; i<ccount; ++i)
		{
		h = h1 + (i*dh+rounder)/diver;
		l = l1 + (i*dl+rounder)/diver;
		s = s1 + (i*ds+rounder)/diver;
		hls_to_rgb(&r,&g,&b,h,l,s);
		dr->r = r;
		dr->g = g;
		dr->b = b;
		++dr;
		}
}


#ifdef SLUFFED
int a_break_key(void)
{
	switch ((UBYTE)icb.inkey)
	{
		case 'x':
		case 'X':
		case 'q':
		case 'Q':
		case ' ':
			return(1);
	}
	return(0);
}
#endif /* SLUFFED */

static void get_menu_5(void)
{
Rgb3 omc[NUM_MUCOLORS];

	/* save old interface colors */
	pj_copy_bytes(vconfg.mc_ideals, omc, sizeof(omc) );

	/* use the last 5 slots in user color map for ideals and 
     * ( will force menu refresh) */

	set_new_mucolors(&(vb.pencel->cmap->ctab[FIRST_MUCOLOR]),vb.screen);
	hide_mp();   /* force redraw of menus */
	show_mp();

	/* if visible query user... */

	if(visible_cmap())
	{
		if(!soft_yes_no_box("use_mc" ))
			set_new_mucolors(omc,vb.screen); /* restore old colors */
	}
	else /* restore and display message */
	{
		set_new_mucolors(omc,vb.screen);
		soft_continu_box("bad_colors");
	}
}


/* Some RGB values for common colors */
Rgb3 pure_white = {255,255,255};
Rgb3 pure_black = {0,0,0};

Boolean visible_cmap(void)
/* Are colors distinct enough from each other? */
{
	return(visible_mucolors(vb.screen->wndo.cmap,vb.screen->mc_colors));
}
void find_colors(void)
{
	find_mucolors(vb.screen);
}

#define IDC 6

static Rgb3 cideals[IDC][NUM_MUCOLORS] =      /* menu colors to try for */
{
	/** steel **/
	{
		{0, 0, 0},	/* menu black */
		{22*4, 22*4, 22*4}, /* menu grey */
		{38*4, 38*4, 38*4}, /* menu light */
		{52*4, 52*4, 52*4}, /* menu bright */
		{63*4, 0, 0},  /* menu red */
	},
	/* ocean */
	{
		{0, 0, 164},	/* menu black */
		{80, 86, 255}, /* menu grey */
		{146, 146, 255}, /* menu light */
		{0, 255, 255}, /* menu bright */
		{255, 0, 0},  /* menu red */
	},
	/** classic **/
	{
		{104, 104, 104},	/* menu black */
		{48, 48, 48}, /* menu grey */
		{8, 8, 8}, /* menu light */
		{109, 14, 77}, /* menu bright */
		{133, 89, 162},  /* menu red */
	},
	/** burgundy **/
	{
		{109, 14, 77},	/* menu black */
		{12, 60, 204}, /* menu grey */
		{156, 156, 252}, /* menu light */
		{60, 252, 252}, /* menu bright */
		{192, 0, 0},  /* menu red */
	},
	/** spring **/
	{
		{0, 90, 2},	/* menu black */
		{156, 60, 60}, /* menu grey */
		{15, 144, 113}, /* menu light */
		{215, 218, 168}, /* menu bright */
		{255, 0, 0},  /* menu red */
	},
	/** inferno **/
	{
		{123, 0, 0},	/* menu black */
		{255, 29, 0}, /* menu grey */
		{46, 0, 46}, /* menu light */
		{0, 119, 98}, /* menu bright */
		{160, 255, 0},  /* menu red */
	},
};

void get_menu_colors()
{
int choice;

	if((choice = soft_qchoice(NULL, "menu_colors")) < 0)
		return;
	if (choice < IDC)
		set_new_mucolors((Rgb3 *)(cideals+choice), vb.screen);
	else
		get_menu_5();
}


void crestore(void)
{
	hide_mp();
	restore();
	sliders_from_ccolor();
	show_mp();
}

static void ccolor_from_sliders(void)
{
Rgb3 *rgb;
SHORT r, g, b;

	rgb = vb.pencel->cmap->ctab + vs.ccolor;
	if (vs.hls)
	{
		hls_to_rgb(&r, &g, &b, ccred, ccgreen, ccblue);
	}
	else
	{
		r = ccred;
		g = ccgreen;
		b = ccblue;
	}
	rgb->r = r;
	rgb->g = g;
	rgb->b = b;
	wait_sync();
	pj_set_colors(vb.screen, vs.ccolor,1,(UBYTE *)rgb);
}

static void change_hls_mode(Button *m)
{
	change_mode(m);
	sliders_from_ccolor();
	see_color_sliders();
}
static void pal_menu_back(Button *m)
{
	wbg_ncorner_back(m);
}
void cycle_ccolor(void)
/* called when a tool has done it's thing */
{
	vs.cdraw_ix++;
	if (vs.cdraw_ix >= cluster_count())
		vs.cdraw_ix = 0;
	vs.ccolor = vs.buns[vs.use_bun].bundle[vs.cdraw_ix];

	if (vs.ink_id == tlc_INKID || vs.ink_id == tsp_INKID)
	{
		/* only re-make if already up */
		if(vl.ink->needs & INK_CASHE_MADE) 
		{
			free_render_cashes();
			make_render_cashes();
		}
	}
}
void cycle_redraw_ccolor()
{
	cycle_ccolor();
	do_color_redraw(NEW_CCOLOR);
}
void set_ccolor(int newccol)
{
int bun_ix;

	if(vs.ccolor != newccol)
		set_color_redraw(NEW_CCOLOR);

	vs.ccolor = newccol;
	if(vs.cycle_draw)
	{
		if((bun_ix = in_bundle(vs.ccolor,&vs.buns[vs.use_bun])) <= 0)
		{
			vs.cycle_draw = 0;
			set_color_redraw(NEW_CCYCLE);
		}
		else
			vs.cdraw_ix = bun_ix - 1;
	}
}
void update_ccolor(int newcol)
{
	set_ccolor(newcol);
	do_color_redraw(NEW_CCOLOR);
}
void set_use_bun(int newbun)
{
int obun;
	obun = vs.use_bun;
	vs.use_bun = newbun;
	if(obun != newbun)
	{
		if(vs.cycle_draw)
		{
			vs.cycle_draw = 0;
			toggle_ccycle();
		}
	}
}
void set_ccycle(Boolean newcyc)
{
	if((vs.cycle_draw = newcyc) != 0)
	{
		vs.cdraw_ix = 0;
		set_ccolor(vs.buns[vs.use_bun].bundle[0]);
		do_color_redraw(NEW_CCOLOR|NEW_CCYCLE);
	}
}
void toggle_ccycle()
{
	set_ccycle(!vs.cycle_draw);
}
void mb_toggle_ccycle(Button *b)
{
	toggle_ccycle();
	draw_buttontop(b); /* draws twice if ccolor changed oh well */
}
int get_mousecolor(Wndo *w)
{
(void)w;

	if(JSTHIT(MBRIGHT))
		return(check_pen_abort());
	if(JSTHIT(MBPEN))
		set_ccolor(pj_get_dot(vb.screen->viscel, icb.sx, icb.sy));
	return(0); /* no need to reset mouse in do_reqloop() */
}
static void scale_palette_menu(Rscale *scale)
{
	scale_powell_palette(scale);
}

static Boolean pal_dopull(Menuhdr *mh)
{
Boolean cclip_isnt;

	cclip_isnt= !pj_exists(cclip_name);

	set_pul_disable(mh, VAL_USE_PUL, (thecel == NULL));
	set_pul_disable(mh, VAL_BLE_PUL, cclip_isnt);
	set_pul_disable(mh, VAL_PAS_PUL, cclip_isnt);
	pul_xflag(mh, PAL_CYC_PUL, vs.cycle_draw);
	return(menu_dopull(mh));
}

static void qone_palette(void)
{
	if (soft_yes_no_box("one_palette"))
		one_palette();
}

static void palette_selit(Menuhdr *mh, SHORT hitid)
{
(void)mh;

switch(hitid)
	{
	case PAL_RES_PUL:
		crestore();
		break;
	case PAL_CYC_PUL:  /* Cycle Draw */
		toggle_ccycle();
		break;
	case PAL_ONE_PUL:	/* one palette */
		hide_mp();
		qone_palette();
		show_mp();
		break;
	case PAL_MEN_PUL:	/* menu colors */
		get_menu_colors();
		break;
	case PAL_FIL_PUL:
		go_files(3);
		break;
	case CLU_GET_PUL:	/* get range */
		qselect_bundle();
		break;
	case CLU_PIC_PUL: /* pick cluster */
		qpick_bundle();
		break;
	case CLU_UNU_PUL:	/* unused colors */
		cunused();
		break;
	case CLU_LIN_PUL: /* line cluster */
		cluster_line();
		break;
	case CLU_FIN_PUL:	/* find ramp */
		find_ramp();
		break;
	case CLU_NEA_PUL:	/* near colors */
		cclose();
		break;
	case CLU_INV_PUL: /* invert cluster */
		cluster_invert();
		break;
	case CLU_PIN_PUL: /* ping-pong */
		ping_cluster();
		break;
	case CLU_REV_PUL: /* reverse cluster */
		cluster_reverse();
		break;
	case ARR_LUM_PUL:
		csort(); /* luma sort */
		break;
	case ARR_SPE_PUL:
		cspec(); /* spectrums */
		break;
	case ARR_GRA_PUL:
		cthread(); /* gradients */
		break;
	case ARR_CYC_PUL: /* Cycle Palette */
		ccycle();
		break;
	case ARR_TRA_PUL:  /* swap cluster */
		cl_swap();
		break;
	case VAL_SQU_PUL:
		cpack();
		break;
	case VAL_RAM_PUL: /* force ramp */
		force_ramp();
		break;
	case VAL_TIN_PUL:
		ctint();
		break;
	case VAL_NEG_PUL:	/* negative */
		cneg();
		break;
	case VAL_USE_PUL:
		cuse_cel();
		break;
	case VAL_DEF_PUL:
		cdefault();
		break;
	case VAL_CUT_PUL: /* cut */
		cl_cut();
		break;
	case VAL_PAS_PUL: /* paste */
		cl_paste();
		break;
	case VAL_BLE_PUL: /* blend */
		cl_blend();
		break;
	}
}



static UBYTE pal_disable;
void disable_palette_menu()
{
	++pal_disable;
}
void enable_palette_menu()
{
	--pal_disable;
}
void palette(void)
{
Menuhdr tpull;
Cursorhdr *och;
void *oundo, *oredo;
void *ss;

	if(MENU_ISOPEN(&palette_menu))
		return;
	if(pal_disable > 0)
		return;

	scale_palette_menu(&vb.screen->menu_scale);
	if (soft_buttons("palette_panel", pal_blist, 
					 Array_els(pal_blist), &ss) < Success)
	{
		return;
	}

	flx_clear_olays();
	oundo = vl.undoit;
	oredo = vl.redoit;
	if (oundo != NULL)	/* in case came from anim cel menu... */
		vl.undoit = swap_undo;
	vl.redoit = NULL;

	fliborder_on();
	menu_to_quickcent(&palette_menu);
	och = set_pen_cursor(&menu_cursor.hdr);
	if (load_soft_pull(&tpull, 8, "palette", PALP_MUID,
		palette_selit, pal_dopull) >= Success)
	{
		do_reqloop(vb.screen,&palette_menu,NULL,&tpull,get_mousecolor);
		smu_free_pull(&tpull);
	}

	/* KLUDGE re-sync undo if there was no undo function */
	if(vl.undoit == NULL) 
		save_undo();
	vl.undoit = oundo;
	vl.redoit = oredo;
	if(dirty_frame)
		do_color_redraw(NEW_CMAP);
	set_pen_cursor(och);
	flx_draw_olays();
	smu_free_scatters(&ss);
}
void ppalette(void)
{
	if(pal_disable > 0)
		return;
	hide_mp();
	palette();
	show_mp();
}
