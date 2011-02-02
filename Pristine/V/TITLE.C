/* title.c - The data structures and associated routines for the Titling
   control panel. */


/* generated with makemenu */
#include "jimk.h"
#include "flicmenu.h"
#include "fli.h"
#include "gemfont.h"
#include "text.h"
#include "commonst.h"
#include "title.str"


/*** Display Functions ***/
extern ccorner_text(), ncorner_text(), left_text(), gary_menu_back(),
	move_tab_text(), title_back();



/*** Select Functions ***/
extern toggle_group(), change_mode(), move_menu(), qedit_text(),
	bottom_menu(), title_button(), suggest_frames(),
	mload_text(), mload_font(), qplace_window(), qnew_text();


/*** Button Data ***/
static Flicmenu tit_fil_sel = {
	NONEXT,
	NOCHILD,
	253, 175, 65, 6,
	title_100 /* "fill line" */,
	left_text,
	change_mode,
	&vs.tit_just,3,
	NOKEY,
	NOOPT,
	0,
	};
static Flicmenu tit_cen_sel = {
	&tit_fil_sel,
	NOCHILD,
	253, 167, 65, 6,
	title_101 /* "center" */,
	left_text,
	change_mode,
	&vs.tit_just,2,
	NOKEY,
	NOOPT,
	0,
	};
static Flicmenu tit_rig_sel = {
	&tit_cen_sel,
	NOCHILD,
	253, 159, 65, 6,
	title_102 /* "right" */,
	left_text,
	change_mode,
	&vs.tit_just,1,
	NOKEY,
	NOOPT,
	0,
	};
static Flicmenu tit_lef_sel = {
	&tit_rig_sel,
	NOCHILD,
	253, 151, 65, 6,
	title_103 /* "left" */,
	left_text,
	change_mode,
	&vs.tit_just,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tit_fra_sel = {
	&tit_lef_sel,
	NOCHILD,
	174, 183, 77, 6,
	title_104 /* "frame count" */,
	left_text,
	suggest_frames,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tit_byc_sel = {
	&tit_fra_sel,
	NOCHILD,
	174, 159, 77, 6,
	title_105 /* "by character" */,
	left_text,
	change_mode,
	&vs.tit_scroll,1,
	NOKEY,
	NOOPT,
	};
static Flicmenu tit_byp_sel = {
	&tit_byc_sel,
	NOCHILD,
	174, 151, 77, 6,
	title_106 /* "by pixel" */,
	left_text,
	change_mode,
	&vs.tit_scroll,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tit_sti_sel = {
	&tit_byp_sel,
	NOCHILD,
	84, 175, 88, 6,
	title_107 /* "still" */,
	left_text,
	change_mode,
	&vs.tit_move,3,
	NOKEY,
	NOOPT,
	0,
	};
static Flicmenu tit_typ_sel = {
	&tit_sti_sel,
	NOCHILD,
	84, 167, 88, 6,
	title_108 /* "type on" */,
	left_text,
	change_mode,
	&vs.tit_move,2,
	NOKEY,
	NOOPT,
	0,
	};
static Flicmenu tit_acr_sel = {
	&tit_typ_sel,
	NOCHILD,
	84, 159, 88, 6,
	title_109 /* "scroll across" */,
	left_text,
	change_mode,
	&vs.tit_move,1,
	NOKEY,
	NOOPT,
	};
static Flicmenu tit_ups_sel = {
	&tit_acr_sel,
	NOCHILD,
	84, 151, 88, 6,
	title_110 /* "scroll up" */,
	left_text,
	change_mode,
	&vs.tit_move,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tit_fon_sel = {
	&tit_ups_sel,
	NOCHILD,
	1, 191, 81, 6,
	title_111 /* "load font" */,
	left_text,
	mload_font,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tit_pla_sel = {
	&tit_fon_sel,
	NOCHILD,
	1, 183, 81, 6,
	title_112 /* "place window" */,
	left_text,
	qplace_window,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tit_loa_sel = {
	&tit_pla_sel,
	NOCHILD,
	1, 175, 81, 6,
	title_113 /* "load text" */,
	left_text,
	mload_text,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tit_edi_sel = {
	&tit_loa_sel,
	NOCHILD,
	1, 167, 81, 6,
	title_114 /* "edit text" */,
	left_text,
	qedit_text,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tit_new_sel = {
	&tit_edi_sel,
	NOCHILD,
	1, 159, 81, 6,
	title_115 /* "new text" */,
	left_text,
	qnew_text,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tit_do__sel = {
	&tit_new_sel,
	NOCHILD,
	1, 151, 81, 6,
	title_116 /* "do titling" */,
	left_text,
	title_button,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tit_jus_sel = {
	&tit_do__sel,
	NOCHILD,
	252, 140, 42, 6,
	title_117 /* "justify" */,
	left_text,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tit_scr_sel = {
	&tit_jus_sel,
	NOCHILD,
	173, 140, 54, 6,
	title_118 /* "scrolling" */,
	left_text,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tit_mov_sel = {
	&tit_scr_sel,
	NOCHILD,
	86, 140, 46, 6,
	title_119 /* "movement" */,
	left_text,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tit_tit_sel = {
	&tit_mov_sel,
	NOCHILD,
	4, 139, 50, 8,
	title_120 /* "titling" */,
	move_tab_text,
	move_menu,
	NOGROUP,0,
	NOKEY,
	bottom_menu,
	};
static Flicmenu tit_menu = {
	NONEXT,
	&tit_tit_sel,
	0, 137, 319, 62,
	NOTEXT,
	title_back,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};

static
title_disables()
{
tit_do__sel.disabled = !jexists(text_name);
}

static
title_back(m)
Flicmenu *m;
{
gary_menu_back(m);
title_disables();
}


do_title_menu()
{
int i;

unzoom();
grab_usr_font();
clip_rmove_menu(&tit_menu, 
	quick_menu.x - tit_menu.x, quick_menu.y-tit_menu.y); 
do_menu(&tit_menu);
free_cfont();
rezoom();
}


/* Start of code not particularly concerned with menu */

static
qplace_window()
{
qpwt(0);
}

static
mload_font()
{
hide_mp();
qfont_text();
draw_mp();
}

static
qnew_text()
{
hide_mp();
save_undo();
if (cut_cursor())
	ttool(0);
draw_mp();
}

static int fc_height;
static long pixels_to_scroll;

extern render_bitmap_blit();

static
title1(ix, intween, scale)
int ix, intween, scale;
{
long tscroll;
int lscroll, pscroll;
int extra;

tscroll = pixels_to_scroll * scale / SCALE_ONE;
lscroll = tscroll/fc_height;
if (vs.tit_scroll == 1)	/* character scrolling */
	{
	pscroll = 0;
	extra = 0;
	}
else
	{
	pscroll = tscroll - (long)lscroll*fc_height;
	extra = 2*fc_height;
	}
render_grad_twin();
if (make_render_cashes())
	{
	wwtext(render_form, usr_font, text_buf, 
		vs.twin_x, vs.twin_y - pscroll, vs.twin_w, vs.twin_h + extra, 
		vs.ccolor, render_bitmap_blit, lscroll, vs.tit_just);
	free_render_cashes();
	return(1);
	}
else
	return(0);
}


/* Scroll a single line of text from right to left */
static
timesq1(ix, intween, scale)
int ix, intween, scale;
{
long tscroll;
long so_far;
char *s;
char c;
int cw;
int ow;
int poff;

tscroll = pixels_to_scroll * scale / SCALE_ONE;
s = text_buf;
so_far = 0;
for (;;)
	{
	c = *s;
	if (c == 0)
		return(1);		/* damn, scrolled off already! */
	cw = fchar_width(usr_font,s);
	so_far += cw;
	if (so_far > tscroll)
		{
		so_far -= cw;
		break;
		}
	s++;
	}
if (vs.tit_scroll == 1)	/* character scrolling */
	poff = 0;
else
	poff = tscroll - so_far;
render_grad_twin();
if (make_render_cashes())
	{
	gftext(render_form, usr_font, s,vs.twin_x - poff, vs.twin_y, vs.ccolor,
		render_bitmap_blit, sblack);
	free_render_cashes();
	return(1);
	}
else
	return(0);
}

static int characters;
static int clines;

extern char *wwnext_line();

static
type1(ix, intween, scale)
int ix, intween, scale;
{
int stoppos;
char *stops;
int scrolls;
char *s, *ls;
char line_buf[256];
int i;
int y;
int ending;

/* figure out which character to stop at */
stoppos = 1+rscale_by(characters, scale, SCALE_ONE);
if (stoppos > characters)
	stoppos = characters;
stops = text_buf + stoppos;

/* calculate how many lines we've scrolled up */
s = text_buf;
scrolls = 0;
for (;;)
	{
	if ((s = wwnext_line(usr_font, s, vs.twin_w, line_buf, 0)) == NULL)
		break;
	scrolls++;
	if (s >= stops)
		break;
	}
scrolls -= clines;
if (scrolls < 0)
	scrolls = 0;

/* skip the lines that've scrolled by already */
s = text_buf;
while (--scrolls >= 0)
	s = wwnext_line(usr_font, s, vs.twin_w, line_buf, 0);

/* now draw the rest. */
render_grad_twin();
if (make_render_cashes())
	{
	y = vs.twin_y;
	i = clines;
	while (--i >= 0)
		{
		ls = s;
		if ((s = wwnext_line(usr_font, s, vs.twin_w, line_buf, 0)) == NULL)
			break;
		ending = (s >= stops);
		if (ending)		/* cut short last line */
			line_buf[stops - ls] = 0;
		justify_line(render_form, usr_font, line_buf, vs.twin_x, y, vs.twin_w, 
			vs.ccolor, render_bitmap_blit, sblack, vs.tit_just);
		y += fc_height;
		if (ending)
			break;
		}
	free_render_cashes();
	return(1);
	}
else
	return(0);
}


static
calc_clines()
{
int h, lines, fh, ch;

lines = 0;
ch = font_cel_height(usr_font);
h = vs.twin_h - usr_font->frm_hgt;
while (h >= 0)
	{
	lines++;
	h -= ch;
	}
return(lines);
}


static
find_pixels_to_scroll()
{
int text_lines;

fc_height = font_cel_height(usr_font);
switch (vs.tit_move)
	{
	case 0:		/* normal vertical scrolling */
		text_lines =  wwcount_lines(usr_font, text_buf, vs.twin_w);
		pixels_to_scroll = text_lines * (long)fc_height;
		break;
	case 1:		/* times square 1 line scrolling */
		tr_string(text_buf, '\n', ' ');	/* convert newlines to space */
		pixels_to_scroll = fstring_width(usr_font,text_buf);
		break;
	case 2:	/* Type on */
		clines = calc_clines();
		characters = strlen(text_buf);
		break;
	case 3: /* still */
		pixels_to_scroll = 0;
		break;
	}
}


static
title_button()	/* aka do text */
{
int omulti, oh;

omulti = vs.multi;
vs.multi = 1;
hide_mp();
push_inks();
ink_push_cel();
render_xmin = vs.twin_x;
if (render_xmin < 0)
	render_xmin = 0;
render_ymin = vs.twin_y;
if (render_ymin < 0)
	render_ymin = 0;
render_xmax = vs.twin_x + vs.twin_w;
if (render_xmax > XMAX)
	render_xmax = XMAX;
render_ymax = vs.twin_y + vs.twin_h;
if (render_ymax > YMAX)
	render_ymax = YMAX;
if (load_text(text_name))
	{
	find_pixels_to_scroll();
	switch (vs.tit_move)
		{
		case 0:		/* normal vertical scrolling */
		case 3:		/* still */
			doauto(title1);
			break;
		case 1:		/* times square 1 line scrolling */
			oh = vs.twin_h;
			vs.twin_h = usr_font->frm_hgt;
			doauto(timesq1);
			vs.twin_h = oh;
			break;
		case 2:	/* Type on */
			if (clines >= 0)
				{
				doauto(type1);
				}
			break;
		}
	free_text();
	}
render_xmin = render_ymin = 0;
render_xmax = XMAX;
render_ymax = YMAX;
ink_pop_cel();
pop_inks();
vs.multi = omulti;
draw_mp();
}

static char *sf_lines[3] = {
	title_121 /* "To get one frame per scroll" */,
};

static
suggest_frames()
{
char buf[40];

hide_mp();
if (load_text(text_name))
	{
	find_pixels_to_scroll();
	switch (vs.tit_move)
		{
		case 0:  /* vertical scroll */
			if (vs.tit_scroll == 1)	/* character scrolling */
				pixels_to_scroll /= fc_height;
			break;
		case 1:  /* horizontal scroll */
			if (vs.tit_scroll == 1)	/* character scrolling */
				pixels_to_scroll = strlen(text_buf) + 
					vs.twin_w/fstring_width(usr_font, cst_space);
			break;
		case 2:	/* type on */
			pixels_to_scroll = strlen(text_buf);
			break;
		case 3: /* still */
			pixels_to_scroll = 1;
			break;
		}
	free_text();
	sprintf(buf, title_123 /* "you'd need %ld frames." */, pixels_to_scroll);
	sf_lines[1] = buf;
	continu_box(sf_lines);
	}
draw_mp();
}



