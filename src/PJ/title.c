/* title.c - The data structures and associated routines for the Titling
   control panel. */

#include "jimk.h"
#include "auto.h"
#include "errcodes.h"
#include "softmenu.h"
#include "textedit.h"
#include "util.h"

Errcode check_max_frames(int count);

static void seebg_title_back(Menuwndo *mw);
static void qplace_window(void);
static void mload_font(void);
static void qnew_text(void);
static void title_button(void);
static void suggest_frames(Button *b);
static void make_suggested_frames(Button *b);

/* generated with makemenu */

/*** Button Data ***/
static Button tit_fil_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	65+1,6+1,253,(175)-137,
	NODATA, /* "Fill Line", */
	black_ltext,
	change_mode,
	NOOPT,
	&vs.tit_just,3,
	NOKEY,
	MB_B_GHILITE
	);
static Button tit_cen_sel = MB_INIT1(
	&tit_fil_sel,
	NOCHILD,
	65+1,6+1,253,(167)-137,
	NODATA, /* "Center", */
	black_ltext,
	change_mode,
	NOOPT,
	&vs.tit_just,2,
	NOKEY,
	MB_B_GHILITE
	);
static Button tit_rig_sel = MB_INIT1(
	&tit_cen_sel,
	NOCHILD,
	65+1,6+1,253,(159)-137,
	NODATA, /* "Right", */
	black_ltext,
	change_mode,
	NOOPT,
	&vs.tit_just,1,
	NOKEY,
	MB_B_GHILITE
	);
static Button tit_lef_sel = MB_INIT1(
	&tit_rig_sel,
	NOCHILD,
	65+1,6+1,253,(151)-137,
	NODATA, /* "Left", */
	black_ltext,
	change_mode,
	NOOPT,
	&vs.tit_just,0,
	NOKEY,
	MB_B_GHILITE
	);
static Button tit_fra_sel = MB_INIT1(
	&tit_lef_sel,
	NOCHILD,
	77+1,6+1,174,(183)-137,
	NODATA, /* "Frame Count", */
	black_ltext,
	suggest_frames,
	make_suggested_frames,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button tit_byc_sel = MB_INIT1(
	&tit_fra_sel,
	NOCHILD,
	77+1,6+1,174,(159)-137,
	NODATA, /* "By Character", */
	black_ltext,
	change_mode,
	NOOPT,
	&vs.tit_scroll,1,
	NOKEY,
	MB_B_GHILITE
	);
static Button tit_byp_sel = MB_INIT1(
	&tit_byc_sel,
	NOCHILD,
	77+1,6+1,174,(151)-137,
	NODATA, /* "By Pixel", */
	black_ltext,
	change_mode,
	NOOPT,
	&vs.tit_scroll,0,
	NOKEY,
	MB_B_GHILITE
	);
static Button tit_sti_sel = MB_INIT1(
	&tit_byp_sel,
	NOCHILD,
	88+1,6+1,84,(175)-137,
	NODATA, /* "Still", */
	black_ltext,
	change_mode,
	NOOPT,
	&vs.tit_move,3,
	NOKEY,
	MB_B_GHILITE
	);
static Button tit_typ_sel = MB_INIT1(
	&tit_sti_sel,
	NOCHILD,
	88+1,6+1,84,(167)-137,
	NODATA, /* "Type On", */
	black_ltext,
	change_mode,
	NOOPT,
	&vs.tit_move,2,
	NOKEY,
	MB_B_GHILITE
	);
static Button tit_acr_sel = MB_INIT1(
	&tit_typ_sel,
	NOCHILD,
	88+1,6+1,84,(159)-137,
	NODATA, /* "Scroll Across", */
	black_ltext,
	change_mode,
	NOOPT,
	&vs.tit_move,1,
	NOKEY,
	MB_B_GHILITE
	);
static Button tit_ups_sel = MB_INIT1(
	&tit_acr_sel,
	NOCHILD,
	88+1,6+1,84,(151)-137,
	NODATA, /* "Scroll Up", */
	black_ltext,
	change_mode,
	NOOPT,
	&vs.tit_move,0,
	NOKEY,
	MB_B_GHILITE
	);
static Button tit_fon_sel = MB_INIT1(
	&tit_ups_sel,
	NOCHILD,
	81+1,6+1,1,(191)-137,
	NODATA, /* "Load Font", */
	black_ltext,
	mload_font,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button tit_pla_sel = MB_INIT1(
	&tit_fon_sel,
	NOCHILD,
	81+1,6+1,1,(183)-137,
	NODATA, /* "Place Window", */
	black_ltext,
	qplace_window,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);

static go_text_files(Button *b)
{
go_files(4);
}

static Button tit_loa_sel = MB_INIT1(
	&tit_pla_sel,
	NOCHILD,
	81+1,6+1,1,(175)-137,
	NODATA, /* "Text Files...", */
	black_ltext,
	go_text_files,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
extern void qedit_titles();
static Button tit_edi_sel = MB_INIT1(
	&tit_loa_sel,
	NOCHILD,
	81+1,6+1,1,(167)-137,
	NODATA, /* "Edit Text", */
	black_ltext,
	qedit_titles,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button tit_new_sel = MB_INIT1(
	&tit_edi_sel,
	NOCHILD,
	81+1,6+1,1,(159)-137,
	NODATA, /* "New Text", */
	black_ltext,
	qnew_text,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button tit_do__sel = MB_INIT1(
	&tit_new_sel,
	NOCHILD,
	81+1,6+1,1,(151)-137,
	NODATA, /* "Do Titling", */
	black_ltext,
	title_button,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button tit_jus_sel = MB_INIT1(
	&tit_do__sel,
	NOCHILD,
	42+1,6+1,252,(140)-137,
	NODATA, /* "Justify", */
	black_ltext,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button tit_scr_sel = MB_INIT1(
	&tit_jus_sel,
	NOCHILD,
	54+1,6+1,173,(140)-137,
	NODATA, /* "Scrolling", */
	black_ltext,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button tit_mov_sel = MB_INIT1(
	&tit_scr_sel,
	NOCHILD,
	46+1,6+1,86,(140)-137,
	NODATA, /* "Movement", */
	black_ltext,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);

static Button tit_tit_sel = MB_INIT1(
	&tit_mov_sel,
	NOCHILD,
	75,8+1,4,(139)-137,
	NODATA, /* "Titling", */
	see_titlebar,
	feel_titlebar,
	mb_menu_to_bottom,
	&tbg_moveclose,0,
	'q',
	0
	);

void seebg_title_back();

static Menuhdr tit_menu = {
	{320,62,0,137,},   /* width, height, x, y */
	TITLE_MUID,		/* id */
	PANELMENU,		/* type */
	&tit_tit_sel,	/* buttons */
	SCREEN_FONT,	/* font */
	&menu_cursor,	/* cursor */
	seebg_title_back, 		/* seebg */
	NULL,					/* dodata */
	NULL,					/* domenu */
	(MBPEN|MBRIGHT|KEYHIT), /* ioflags */
};

static Smu_button_list tit_smblist[] = {
	{ "title", &tit_tit_sel },
	{ "fill", &tit_fil_sel },
	{ "center", &tit_cen_sel },
	{ "right", &tit_rig_sel },
	{ "left", &tit_lef_sel },
	{ "fcount", &tit_fra_sel },
	{ "by_char", &tit_byc_sel },
	{ "By_pix", &tit_byp_sel },
	{ "still", &tit_sti_sel },
	{ "type_on", &tit_typ_sel },
	{ "across", &tit_acr_sel },
	{ "scroll_up", &tit_ups_sel },
	{ "ld_font", &tit_fon_sel },
	{ "movewin", &tit_pla_sel },
	{ "ld_text", &tit_loa_sel },
	{ "edit", &tit_edi_sel },
	{ "new", &tit_new_sel },
	{ "render", &tit_do__sel },
	{ "justify", &tit_jus_sel },
	{ "scroll", &tit_scr_sel },
	{ "movement", &tit_mov_sel },
};

static void title_disables(void)
{
 	set_button_disable(&tit_do__sel,!pj_exists(text_name));
}

static void seebg_title_back(Menuwndo *mw)
{
	seebg_white(mw);
	title_disables();
}

void do_title_menu(void)
{
void *ss;

	unzoom();
	hide_mp();

	if (soft_buttons("titles_panel", tit_smblist, 
					 Array_els(tit_smblist), &ss) < Success)
	{
		goto error;
	}
	menu_to_quickcent(&tit_menu);
	do_reqloop(vb.screen,&tit_menu,NULL,NULL,NULL);
	smu_free_scatters(&ss);
error:
	show_mp();
	rezoom();
}

/* Start of code not particularly concerned with menu */

static void qplace_window(void)
{
qpwtitles(0);
}

static void mload_font(void)
{
hide_mp();
qfont_text();
show_mp();
}

static void qnew_text(void)
{
hide_mp();
save_undo();
if (marqi_cut_xy() >= 0)
	ttool(0);
show_mp();
}

static int fc_height;
static long pixels_to_scroll;

static int title1(Text_file *gf,int ix,int intween,int scale)
{
Errcode err;
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
	set_twin_gradrect();

	if((err = make_render_cashes()) < 0)
		goto error;

	wwtext(vb.pencel, uvfont, gf->text_buf, 
		vs.twin.x, vs.twin.y - pscroll, vs.twin.width, vs.twin.height + extra, 
		lscroll, vs.tit_just, vs.ccolor, TM_RENDER, sblack);

	free_render_cashes();

error:
	return(err);
}


static int timesq1(Text_file *gf,int ix,int intween,int scale)
/* Scroll a single line of text from right to left */
{
Errcode err;
long tscroll;
long so_far;
char *s;
char c;
int cw;
int poff;

	tscroll = pixels_to_scroll * scale / SCALE_ONE;
	s = gf->text_buf;
	so_far = 0;
	for (;;)
	{
		c = *s;
		if (c == 0)
			return(0);		/* all done, damn, scrolled off already! */

		cw = fchar_spacing(uvfont,s);
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
	set_twin_gradrect();
	if((err = make_render_cashes()) < 0)
		goto error;

	gftext(vb.pencel, uvfont, s,vs.twin.x - poff, vs.twin.y, vs.ccolor,
		TM_RENDER, sblack);
	free_render_cashes();
error:
	return(err);
}

static int characters;
static int clines;

static int type1(Text_file *gf,int ix,int intween,int scale)
{
Errcode err;
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
	stops = gf->text_buf + stoppos;

	/* calculate how many lines we've scrolled up */
	s = gf->text_buf;
	scrolls = 0;
	for (;;)
	{
		wwnext_line(uvfont, &s, vs.twin.width, line_buf, 0);
		scrolls++;
		if(s == NULL)
			break;
		if (s >= stops)
			break;
	}
	scrolls -= clines;
	if (scrolls < 0)
		scrolls = 0;

	/* skip the lines that've scrolled by already */
	s = gf->text_buf;
	while (--scrolls >= 0)
		wwnext_line(uvfont, &s, vs.twin.width, line_buf, 0);

	/* now draw the rest. */
	set_twin_gradrect();
	if((err = make_render_cashes()) < 0)
		goto error;

	y = vs.twin.y;
	i = clines;
	while (--i >= 0)
	{
		ls = s;
		wwnext_line(uvfont, &s, vs.twin.width, line_buf, 0);
		if(s == NULL)
			ending = TRUE;
		else
			ending = (s >= stops);

		if (ending)		/* cut short last line */
			line_buf[stops - ls] = 0;

		justify_line(vb.pencel, uvfont, line_buf,
					 vs.twin.x, y, vs.twin.width, 
					 vs.ccolor, TM_RENDER, sblack, vs.tit_just,NULL,0);
		y += fc_height;
		if (ending)
			break;
	}
	free_render_cashes();
error:
	return(err);
}


static int calc_clines(void)
{
int h, lines, ch;

lines = 0;
ch = font_cel_height(uvfont);
h = vs.twin.height - tallest_char(uvfont);
while (h >= 0)
	{
	lines++;
	h -= ch;
	}
return(lines);
}


static void find_pixels_to_scroll(Text_file *gf)
{
int text_lines;
SHORT maxwid;

fc_height = font_cel_height(uvfont);
switch (vs.tit_move)
	{
	case 0:		/* normal vertical scrolling */
		text_lines =  wwcount_lines(uvfont, gf->text_buf, 
								    vs.twin.width, &maxwid);
		pixels_to_scroll = text_lines * (long)fc_height;
		break;
	case 1:		/* times square 1 line scrolling */
		tr_string(gf->text_buf, '\n', ' ');	/* convert newlines to space */
		pixels_to_scroll = fstring_width(uvfont,gf->text_buf);
		break;
	case 2:	/* Type on */
		clines = calc_clines();
		characters = strlen(gf->text_buf);
		break;
	case 3: /* still */
		pixels_to_scroll = 0;
		break;
	}
}

static fresh_load_tf(Text_file *gf)
{
	clear_struct(gf);
	return(load_text_file(gf, text_name));
}



Errcode do_titles(Boolean	with_menu)	/* aka do text */
{
int omulti, oh;
Text_file lgtf;
Autoarg aa;
Errcode (*auto_func)(Autoarg *aa);
Errcode err;


	auto_func = (with_menu ? do_auto : noask_do_auto_time_mode);
	omulti = vs.multi;
	vs.multi = 1;
	hide_mp();
	push_inks();
	ink_push_cel();

	set_render_clip(&vs.twin);

	if ((err = fresh_load_tf(&lgtf)) >= 0)
	{
		clear_struct(&aa);
		aa.avecdat = &lgtf;
		aa.flags = 0;
		find_pixels_to_scroll(&lgtf);
		switch (vs.tit_move)
		{
			case 0:		/* normal vertical scrolling */
			case 3:		/* still */
				aa.avec = title1;
				err = (*auto_func)(&aa);
				break;
			case 1:		/* times square 1 line scrolling */
				oh = vs.twin.height;
				vs.twin.height = tallest_char(uvfont);
				aa.avec = timesq1;
				err = (*auto_func)(&aa);
				vs.twin.height = oh;
				break;
			case 2:	/* Type on */
				if (clines >= 0)
				{
					aa.avec = type1;
					err = (*auto_func)(&aa);
				}
				break;
		}
		free_text_file(&lgtf);
	}
	set_render_clip(NULL); /* restore render clip to full screen */
	ink_pop_cel();
	pop_inks();
	vs.multi = omulti;
	show_mp();
	return err;
}

static void title_button(void)	/* aka do text */
{
	do_titles(TRUE);
}

static int calc_suggest_frames()
/* Figure out how many frames it'd take for the titling effect to
 * have one frame per scroll. */
{
Errcode err;
Text_file lgtf;

	if ((err = fresh_load_tf(&lgtf)) >= 0)
	{
		find_pixels_to_scroll(&lgtf);
		switch (vs.tit_move)
		{
			case 0:  /* vertical scroll */
				if (vs.tit_scroll == 1)	/* character scrolling */
					pixels_to_scroll /= fc_height;
				break;
			case 1:  /* horizontal scroll */
				if (vs.tit_scroll == 1)	/* character scrolling */
					pixels_to_scroll = strlen(lgtf.text_buf) + 
						vs.twin.width/fstring_width(uvfont, " ");
				break;
			case 2:	/* type on */
				pixels_to_scroll = strlen(lgtf.text_buf);
				break;
			case 3: /* still */
				pixels_to_scroll = 1;
				break;
		}
		free_text_file(&lgtf);
		return(pixels_to_scroll);
	}
	else
		return(err);
}

static void suggest_frames(Button *b)
{
int pix;

	hide_mp();
	if ((pix = calc_suggest_frames()) > 0)
		soft_continu_box("!%d", "one_per_scroll",pixels_to_scroll);
	show_mp();
}

static void make_suggested_frames(Button *b)
{
short pix;

	hide_mp();
	if ((pix = calc_suggest_frames()) > 0)
		{
		if (check_max_frames(pix) >= Success)
			if (soft_yes_no_box("!%d", "one_scroll", pix))
				set_frame_count(pix);
		}
	show_mp();
}



