/* generated with makemenu 2.0 */
#include <stdio.h>
#include <string.h>
#include "jimk.h"
#include "commonst.h"
#include "errcodes.h"
#include "fontdev.h"
#include "linklist.h"
#include "memory.h"
#include "menus.h"
#include "rastext.h"
#include "reqlib.h"
#include "scroller.h"
#include "softmenu.h"
#include "wildlist.h"

typedef struct qfont_cb
	{
	char fpath[PATH_SIZE];
	char *wildcard; /* wildcard for scroller */
	SHORT *ptop_name;
	Vfont vfont;
	Errcode font_err;
	SHORT point_size;
	SHORT unzag_flag;
	Wscreen *screen;
	} Qfont_cb;
Qfont_cb *qfcb;

static void init_font_scroller(void);
static void free_qfont_font(void);

/*** Display Functions ***/
static void font_sample_text(Button *m);
static void see_font_spacing(Button *m);
static void see_font_leading(Button *m);
static void see_font_height(Button *m);
static void set_font_height(Button *m);
static void see_font_unzag(Button *m);
static void set_font_unzag(Button *m);


/*** Select Functions ***/
static void get_any_font(Button *m);
static void set_font_spacing(Button *m);
static void set_font_leading(Button *m);


extern Image ctriup, ctridown;

static Name_scroller font_scroller;

/*** Button Data ***/
static Button fmu_can_sel =  MB_INIT1(
	NONEXT,
	NOCHILD,
	49,15,268,79,
	NODATA,		/* Read in from menu file */
	dcorner_text,
	mb_close_cancel,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button fmu_ok_sel =  MB_INIT1(
	&fmu_can_sel,
	NOCHILD,
	49,15,216,79,
	NODATA,		/* Read in from menu file */
	dcorner_text,
	mb_close_ok,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button fmu_sam_sel =  MB_INIT1(
	&fmu_ok_sel,
	NOCHILD,
	129,74,99,3,
	NOTEXT,
	font_sample_text,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static void see_font_name(Button *b)
{
	b->datme = pj_get_path_name(qfcb->fpath);
	black_ctext(b);
}
static Button fmu_nam_sel =  MB_INIT1(
	&fmu_sam_sel,
	NOCHILD,
	79,15,235,7-2,
	NULL,
	see_font_name,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button fmu_tdi_sel =  MB_INIT1(
	&fmu_nam_sel,
	NOCHILD,
	114,15,99,79,
	NODATA,		/* Read in from menu file */
	dcorner_text,
	get_any_font,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button fmu_unz_sel =  MB_INIT1(
	&fmu_tdi_sel,
	NOCHILD,
	50,11,250,61,
	NODATA,		/* Read in from menu file */
	see_font_unzag,
	set_font_unzag,
	set_font_unzag,
	NOGROUP,1,
	NOKEY,
	MB_GHILITE
	);
static Button fmu_she_sel =  MB_INIT1(
	&fmu_unz_sel,
	NOCHILD,
	23,11,288,54-6-2,
	NOTEXT,
	see_font_height,
	set_font_height,
	set_font_height,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button fmu_hei_sel =  MB_INIT1(
	&fmu_she_sel,
	NOCHILD,
	47,11,234,54-6-2,
	NODATA,		/* Read in from menu file */
	grey_ctext,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button fmu_ssp_sel =  MB_INIT1(
	&fmu_hei_sel,
	NOCHILD,
	23,11,288,42-6-2,
	NOTEXT,
	see_font_spacing,
	set_font_spacing,
	set_font_spacing,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button fmu_spa_sel =  MB_INIT1(
	&fmu_ssp_sel,
	NOCHILD,
	47,11,234,42-6-2,
	NODATA,		/* Read in from menu file */
	grey_ctext,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button fmu_sle_sel =  MB_INIT1(
	&fmu_spa_sel,
	NOCHILD,
	23,11,288,30-6-2,
	NOTEXT,
	see_font_leading,
	set_font_leading,
	set_font_leading,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button fmu_lea_sel =  MB_INIT1(
	&fmu_sle_sel,
	NOCHILD,
	47,11,234,30-6-2,
	NODATA,		/* Read in from menu file */
	grey_ctext,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button fmu_lis_sel =  MB_INIT1(
	&fmu_lea_sel,
	NOCHILD,
	78,78,17,17,
	&font_scroller,
	see_scroll_names,
	feel_scroll_cels,
	NOOPT,
	&font_scroller,0,
	NOKEY,
	0
	);
static Button fmu_dow_sel =  MB_INIT1(
	&fmu_lis_sel,
	NOCHILD,
	13,11,3,84,
	&ctridown,
	ccorner_image,
	scroll_incdown,
	NOOPT,
	&font_scroller,-1,
	DARROW,
	0
	);
static Button fmu_scr_sel =  MB_INIT1(
	&fmu_dow_sel,
	NOCHILD,
	11,58,4,27,
	&font_scroller,
	see_scrollbar,
	rt_feel_scrollbar,
	NOOPT,
	&font_scroller,0,
	NOKEY,
	0
	);
static Button fmu_ups_sel =  MB_INIT1(
	&fmu_scr_sel,
	NOCHILD,
	13,11,3,17,
	&ctriup,
	ccorner_image,
	scroll_incup,
	NOOPT,
	&font_scroller,-1,
	UARROW,
	0
	);
static Button fmu_tit_sel =  MB_INIT1(
	&fmu_ups_sel,
	NOCHILD,
	92,12,3,3,
	NODATA,		/* Read in from menu file */
	see_titlebar,
	feel_titlebar,
	mb_menu_to_bottom,
	&tbg_moveclose,0,
	NOKEY,
	0
	);
Menuhdr fmu_menu = {
	{320,97,0,0,},
	0,				/* id */
	PANELMENU,		/* type */
	&fmu_tit_sel,	/* buttons */
	SCREEN_FONT,	/* font */
	SCREEN_CURSOR,			/* cursor */
	seebg_white,	/* seebg */
	NULL,			/* data */
	NULL,			/* domenu */
	(MBPEN|MBRIGHT|KEYHIT),	/* ioflags */
};

/*********   End of makemenu generated code *********/

static Smu_button_list font_blist[] = {
	{ "cancel", { &fmu_can_sel } },
	{ "ok",     { &fmu_ok_sel } },
	{ "dir",    { &fmu_tdi_sel } },
	{ "space",  { &fmu_spa_sel } },
	{ "lead",   { &fmu_lea_sel } },
	{ "height", { &fmu_hei_sel } },
	{ "title",  { &fmu_tit_sel } },
	{ "unzag",  { &fmu_unz_sel } },
};


static void make_samp_string(char *p, int start, int count)
{
while (--count >= 0)
	*p++ = start++;
*p++ = 0;
}

static void font_sample_text(Button *m)
/* Print a line of lower case, a line of upper case, and a line of numbers
   (space permitting).  Always print lower case even if it won't fit. */
{
int h;
int x,y;
int i;
int lines;
Clipbox cb;
char buf[27];
static int starts[] = {'a', 'A', '0'};
static int counts[] = {26, 26, 10};

wbg_ncorner_back(m);
if (qfcb->font_err == Success)
	{
	pj_clipbox_make(&cb, (Raster *)m->root, m->x+2, m->y+2, 
		m->width-4, m->height-4);
	h = font_cel_height(&qfcb->vfont);
	lines = m->height/h + 1;
	if (lines > 3)
		lines = 3;
	y = ((m->height - 4 - (lines-1)*h - tallest_char(&qfcb->vfont))>>1);
	if (y < 0)
		y = 0;
	for (i=0; i<lines; i++)
		{
		make_samp_string(buf, starts[i], counts[i]);
		x = ((m->width-4-fstring_width(&qfcb->vfont, buf))>>1);
		if (x < 0)
			x = 0;
		gftext(&cb, &qfcb->vfont, buf, x,  y, mc_grey(m), TM_MASK1, 0);
		y += h;
		}
	}
}

static void see_some_short(Button *m, SHORT *num)
{
	char buf[16];

	if (qfcb->font_err == Success)
		sprintf(buf, "%d", *num);
	else
		buf[0] = 0;
	m->datme = buf;
	dcorner_text(m);
}

static void see_font_height(Button *m)
{
	see_some_short(m, &qfcb->vfont.image_height);
}

static void see_font_spacing(Button *m)
{
	see_some_short(m, &qfcb->vfont.spacing);
}

static void see_font_leading(Button *m)
{
	see_some_short(m, &qfcb->vfont.leading);
}

static void set_font_height(Button *m)
{
	Vfont *font = &qfcb->vfont;
	SHORT height = qfcb->point_size;

	if (font->flags & VFF_SCALEABLE)
		{
		if (soft_qreq_number(&height,20,200,"set_font_height"))
			{
			if (height < 2)
				height = 2;
			if (height > vb.pencel->height)
				height = vb.pencel->height;
			if (fset_height(font, height) >= Success)
				qfcb->point_size = height;
			draw_button(&fmu_sam_sel);
			draw_button(m);
			}
		}
	else
		soft_continu_box("font_scale_only");
}

static void see_font_unzag(Button *m)
{
	Vfont *font = &qfcb->vfont;
	if (font->flags & VFF_SCALEABLE)
		{
		dcorner_text(m);
		}
	else
		{
		white_block(m);
		}
}

static void set_font_unzag(Button *m)
{
	Vfont *font = &qfcb->vfont;
	if (font->flags & VFF_SCALEABLE)
		{
		if (fset_unzag(font, !qfcb->unzag_flag) >= Success)
			{
			toggle_bgroup(m);
			draw_button(&fmu_sam_sel);
			}
		}
	else
		soft_continu_box("font_scale_only");
}

static Errcode update_font_spacing(void *f, SHORT spacing)
{
	fset_spacing((Vfont *)f, spacing, -1);
	draw_button(&fmu_sam_sel);
	return(Success);
}

static void set_font_spacing(Button *m)
{
SHORT ospacing;
SHORT spacing;
Vfont *f = &qfcb->vfont;

	spacing = ospacing = f->spacing;

	if(!clip_soft_qreq_number(&spacing,0,f->widest_image*5,
					          update_font_spacing, f, "font_spacing"))
	{
		spacing = ospacing;
	}
	update_font_spacing(f,spacing);
	draw_button(m);
}

static Errcode update_font_leading(void *f, SHORT leading)
{
	fset_spacing((Vfont *)f,-1,leading);
	draw_button(&fmu_sam_sel);
	return(Success);
}

static void set_font_leading(Button *m)
{
SHORT leadmin, leadmax;
SHORT oleading;
SHORT leading;
Vfont *f = &qfcb->vfont;

	leadmin = qfcb->vfont.default_leading;
	if (leadmin > 0)
		leadmin = 0;

	leadmax = qfcb->vfont.image_height*5;
	if (leadmax < 100)
		leadmax = 100;

	leading = oleading = f->leading;
	if(!clip_soft_qreq_number(&leading,leadmin,leadmax,
							  update_font_leading, f, "font_leading"))
	{
		leading = oleading;
	}
	update_font_leading(f,leading);
	draw_button(m);
}

static Errcode load_qcb_font(char *fpath)
{
	if ((qfcb->font_err = load_font(fpath, &qfcb->vfont, qfcb->point_size
	, qfcb->unzag_flag))
		>= Success)
		qfcb->point_size = qfcb->vfont.image_height;
	return qfcb->font_err;
}

static void maybe_load_qcb_font(char *fpath)
{
	free_qfont_font();
	if (load_qcb_font(fpath) < Success)
	{
		errline(qfcb->font_err, fpath);
		/* attempt re-load previous font */
		qfcb->font_err
		= load_qcb_font(qfcb->fpath);
	}
	else
	{
		strcpy(qfcb->fpath,fpath); /* copy in good path */
	}
}

static void get_any_font(Button *m)
/* Let user pick any type font from anywhere.  Also set the directory
   for that type of font */
{
char sbuf[50];
char selb[17];
char lpath[PATH_SIZE];  /* buffer for path returned from file requestor */
char *fpath;
(void)m;

	hide_mp();
	free_wild_list(&font_scroller.names);
	if((fpath = pj_get_filename(stack_string("sel_any_font",sbuf),
					 		".FNT;.PFB",
							stack_string("sel_str",selb),
				   			qfcb->fpath, lpath, 
							FALSE, qfcb->ptop_name,
							qfcb->wildcard)) != NULL)
	{
		maybe_load_qcb_font(fpath);
	}
	init_font_scroller();
	show_mp();
}

static void refresh_font(void)
/*
 * When font changes we need to redraw some buttons.
 */
{
	draw_button(&fmu_nam_sel);
	draw_button(&fmu_sam_sel);
	draw_button(&fmu_ssp_sel);
	draw_button(&fmu_sle_sel);
	draw_button(&fmu_she_sel);
	draw_button(&fmu_unz_sel);
}
static void feel_1_font(Button *b,void *rast,int x,int y,Names *entry,
					    int why)
{
char fpath[PATH_SIZE];
(void)rast;
(void)x;
(void)y;

	strcpy(fpath,qfcb->fpath);
	strcpy(pj_get_path_name(fpath), entry->name); 
	maybe_load_qcb_font(fpath);
	refresh_font(); /* Redraw all the buttons that reflect current font */
	if(why == SCR_ENTER)
		mb_close_ok(b);
}
static void init_font_scroller(void)
{
char saved_c;
char *name;

	free_wild_list(&font_scroller.names);
	name = pj_get_path_name(qfcb->fpath);
	saved_c = *name;
	*name = 0;
	build_dir_list(&font_scroller.names,"*.*", FALSE, qfcb->fpath);
	*name = saved_c;
	font_scroller.scroll_sel = &fmu_scr_sel;
	font_scroller.list_sel = &fmu_lis_sel;
	font_scroller.font = qfcb->screen->mufont;
	font_scroller.top_name = *qfcb->ptop_name;
	font_scroller.cels_per_row = 1;
	font_scroller.feel_1_cel = feel_1_font;
	init_name_scroller(&font_scroller,qfcb->screen);
}
static void free_qfont_font(void)
{
	if (qfcb->font_err == Success)
		close_vfont(&qfcb->vfont);
	qfcb->font_err = Err_nogood;
}

Errcode font_req(
	char *font_path, 		/* On Success filled in with new path. Not
							 * changed if canceled out */
	char *wildcard,			/* may be null and it will assume "*.fnt" 
							 * Updated if changed even if canceled out */
	SHORT *ptop_name,       /* may be NULL top name for directory scroller 
							 * Updated if changed even if canceled out */
	SHORT *point_size,      
	Vfont *pfont,			/* On Success this font is replaced... (Make
								sure it points to a real font or to all 
								zeroes. Not changed if canceled */
	Wscreen *screen,		/* Screen to put up requestor on */
	SHORT *punzag_flag)		/* Do we unzag this font? */
{
Errcode mu_err;
Qfont_cb cb;
void *ss;

	clear_struct(&cb);
	if ((mu_err = soft_buttons("font_panel", font_blist, 
						     Array_els(font_blist), &ss)) < Success)
	{
		goto OUT;
	}

	qfcb = &cb;

	/* load local buffers from input */
	strcpy(qfcb->fpath,font_path);
	qfcb->wildcard = wildcard;
	qfcb->ptop_name = ptop_name;
	qfcb->point_size = *point_size;
	qfcb->screen = screen;
	qfcb->unzag_flag = *punzag_flag;
	fmu_unz_sel.group = &qfcb->unzag_flag;

	if (load_qcb_font(font_path) == Success)
	{
		fset_spacing(&qfcb->vfont,pfont->spacing,pfont->leading);
	}
	init_font_scroller();
	menu_to_cursor(screen,&fmu_menu);
	mu_err = do_reqloop(screen,&fmu_menu,NULL,NULL,NULL);
	*ptop_name = font_scroller.top_name;
	free_wild_list(&font_scroller.names);
	if (mu_err == Success && qfcb->font_err == Success)
	{
		strcpy(font_path,qfcb->fpath);
		close_vfont(pfont);
		*pfont = qfcb->vfont;
		*point_size = qfcb->point_size;
		*punzag_flag = qfcb->unzag_flag;
	}
	else
	{
		free_qfont_font();
	}
OUT:
	smu_free_scatters(&ss);
	return(mu_err);
}

