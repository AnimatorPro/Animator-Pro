
/* browse.c - Stuff for browse flics.  Shares code with normal file menu
   in filemenu.c in a semi-gnarly fashion.  Wants pstamp.c for
   doing the actual read and shrink of the first frame of a fli. */

#include "jimk.h"
#include "fli.h"
#include "flicmenu.h"
#include "commonst.h"
#include "browse.str"

/* Browse action defines. See browse.c */
#define BA_LOAD 0
#define BA_VIEW 1
#define BA_INFO 2
#define BA_KILL 3
#define BA_PLAY 4

/* size of one browse pic/name combo */
#define BRO_DX 77	
#define BRO_DY 50

extern struct name_list *wild_lst;
extern char wild[];
extern Vector redisplay_drawer;

extern inverse_cursor(), black_block(), see_islidepot(), 
	see_string_req(), ccorner_cursor(), feel_browse(), see_browse(),
	see_number_slider(),  close_menu(), draw_cpi(), draw_cpi_name(),
	qmake_frames(), short_path(),
	mrewind(), mfast_forward(),wcursor(),
	baction_button(),
	mundo_pic(), move_menu(), fill_inkwell(),
	show_sel_mode(), toggle_sel_mode(), see_menu_back(), gary_menu_back(),
	see_range(), see_colors2(), see_ink(),
	pget_color(), ccolor_box(), change_mode(),
	mplayit(), mprev_frame(),mnext_frame(), mfirst_frame(), mlast_frame(),
	text_lineunder(), see_pen(), toggle_pen(), set_pbrush(),
	set_pspeed(), gbnumber_plus1(),
	see_draw_mode(), toggle_draw_mode(), toggle_zoom(),
	bottom_menu(), palette(), options(),text_boxp1(),
	white_frame(), white_block(), hang_child(), go_dmmenu(),
	ncorner_text(),dcorner_text(), ccorner_text(), gary_menu_back(), bcursor(),
	blacktext(), greytext(), grey_block(), toggle_group(),
	gary_menu_ww(), see_scroll(), print_list(),
	fq_drawer_stringq(),
	bfeel_scroller(), new_dev(), go_updir(), go_rootdir(),
	bwtext(), wbtexty1(), wbnumber(), bwnumber(), white_slice();

static bincup(), bincdown();

extern WORD device;

extern struct cursor cdown, cleft, cright, csleft,
	cinsert, cappend, cright2, cleft2, csright, cup, size_cursor,
	ctridown, ctriup, default_c;

static struct name_scroller bscroller;

static char cpi_name[81];
static int browse_ok;	/* result of browse_files() */


/* Data structures for browse buttons */
static Flicmenu bro_cpi_sel = {
	NONEXT,
	NOCHILD,
	103, 156, 65, 41,
	NOTEXT,
	draw_cpi,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	close_menu,
	};
static Flicmenu brw_list_sel = {
	&bro_cpi_sel,
	NOCHILD,
	20,0,298,149,
	(char *)&bscroller,
	see_browse,
	feel_browse,
	NOGROUP, 0,
	NOKEY,
	close_menu,
	};
static Flicmenu brw23 = {
	&brw_list_sel,
	NOCHILD,
	0,136,12,10,
	(char *)&ctridown,
	ccorner_cursor,
	bincdown,
	(WORD *)&bscroller, 0,
	DARROW,
	close_menu,
	};
static Flicmenu brw_scroller_sel = {
	&brw23,
	NOCHILD,
	1,12,10,122,
	&bscroller,
	see_scroll,
	bfeel_scroller,
	NOGROUP, 0,
	NOKEY,
	close_menu,
	};
static Flicmenu brw21 = {
	&brw_scroller_sel,
	NOCHILD,
	0,0,12,10,
	(char *)&ctriup,
	ccorner_cursor,
	bincup,
	(WORD *)&bscroller, 0,
	UARROW,
	close_menu,
	};

/*** Button Data ***/
static Flicmenu bro_pat_sel = {
	&brw21,
	NOCHILD,
	213, 183, 102, 10,
	NULL,
	short_path,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	close_menu,
	};
static Flicmenu bro_tpa_sel = {
	&bro_pat_sel,
	NOCHILD,
	175, 185, 29, 6,
	browse_100 /* "path" */,
	greytext,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	close_menu,
	};
static Flicmenu bro_ldv_sel = {
	&bro_tpa_sel,
	NOCHILD,
	303, 171, 12, 8,
	"l",
	ccorner_text,
	new_dev,
	&device,11,
	NOKEY,
	close_menu,
	};
static Flicmenu bro_kdv_sel = {
	&bro_ldv_sel,
	NOCHILD,
	288, 171, 12, 8,
	"k",
	ccorner_text,
	new_dev,
	&device,10,
	NOKEY,
	close_menu,
	};
static Flicmenu bro_jdv_sel = {
	&bro_kdv_sel,
	NOCHILD,
	273, 171, 12, 8,
	"j",
	ccorner_text,
	new_dev,
	&device,9,
	NOKEY,
	close_menu,
	};
static Flicmenu bro_idv_sel = {
	&bro_jdv_sel,
	NOCHILD,
	258, 171, 12, 8,
	"i",
	ccorner_text,
	new_dev,
	&device,8,
	NOKEY,
	close_menu,
	};
static Flicmenu bro_hdv_sel = {
	&bro_idv_sel,
	NOCHILD,
	243, 171, 12, 8,
	"h",
	ccorner_text,
	new_dev,
	&device,7,
	NOKEY,
	close_menu,
	};
static Flicmenu bro_gdv_sel = {
	&bro_hdv_sel,
	NOCHILD,
	228, 171, 12, 8,
	"g",
	ccorner_text,
	new_dev,
	&device,6,
	NOKEY,
	close_menu,
	};
static Flicmenu bro_fdv_sel = {
	&bro_gdv_sel,
	NOCHILD,
	213, 171, 12, 8,
	"f",
	ccorner_text,
	new_dev,
	&device,5,
	NOKEY,
	close_menu,
	};
static Flicmenu bro_edv_sel = {
	&bro_fdv_sel,
	NOCHILD,
	303, 160, 12, 8,
	"e",
	ccorner_text,
	new_dev,
	&device,4,
	NOKEY,
	close_menu,
	};
static Flicmenu bro_ddv_sel = {
	&bro_edv_sel,
	NOCHILD,
	288, 160, 12, 8,
	"d",
	ccorner_text,
	new_dev,
	&device,3,
	NOKEY,
	close_menu,
	};
static Flicmenu bro_cdv_sel = {
	&bro_ddv_sel,
	NOCHILD,
	273, 160, 12, 8,
	"c",
	ccorner_text,
	new_dev,
	&device,2,
	NOKEY,
	close_menu,
	};
static Flicmenu bro_bdv_sel = {
	&bro_cdv_sel,
	NOCHILD,
	258, 160, 12, 8,
	"b",
	ccorner_text,
	new_dev,
	&device,1,
	NOKEY,
	close_menu,
	};
static Flicmenu bro_adv_sel = {
	&bro_bdv_sel,
	NOCHILD,
	243, 160, 12, 8,
	"a",
	ccorner_text,
	new_dev,
	&device,0,
	NOKEY,
	close_menu,
	};
static Flicmenu bro_roo_sel = {
	&bro_adv_sel,
	NOCHILD,
	228, 160, 12, 8,
	cst_fsep,
	ccorner_text,
	go_rootdir,
	NOFEEL, 0,
	'\\',
	close_menu,
	};
static Flicmenu bro_upd_sel = {
	&bro_roo_sel,
	NOCHILD,
	213, 160, 12, 8,
	"..",
	ccorner_text,
	go_updir,
	NOGROUP, 0,
	'.',
	close_menu,
	};
static Flicmenu bro_tdr_sel = {
	&bro_upd_sel,
	NOCHILD,
	175, 161, 35, 6,
	browse_115 /* "drive" */,
	greytext,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	close_menu,
	};
static Flicmenu bro_ok_sel = {
	&bro_tdr_sel,
	NOCHILD,
	53, 189, 44, 8,
	cst_ok,
	ccorner_text,
	baction_button,
	NOGROUP,0,
	'\r',
	close_menu,
	};
static Flicmenu bro_tna_sel = {
	&bro_ok_sel,
	NOCHILD,
	51, 178, 48, 6,
	cpi_name,
	draw_cpi_name,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	close_menu,
	};
static Flicmenu bro_can_sel = {
	&bro_tna_sel,
	NOCHILD,
	53, 165, 44, 8,
	cst_cancel,
	ccorner_text,
	close_menu,
	NOGROUP,0,
	NOKEY,
	close_menu,
	};
/* dummy entry behind cancel to add key equiv for play ... had to sneak
   it in at the last moment cause real important for presentations. */
static Flicmenu bro_play_sel = {
	&bro_can_sel,
	NOCHILD,
	53, 165, 44, 8,
	NOTEXT,
	NOSEE,
	change_mode,
	&vs.browse_action, BA_PLAY,
	'p',
	close_menu,
	};
static Flicmenu bro_del_sel = {
	&bro_play_sel,
	NOCHILD,
	4, 189, 44, 8,
	browse_118 /* "delete" */,
	ccorner_text,
	change_mode,
	&vs.browse_action, BA_KILL,
	'd',
	close_menu,
	};
static Flicmenu bro_inf_sel = {
	&bro_del_sel,
	NOCHILD,
	4, 181, 44, 8,
	browse_119 /* "info" */,
	ccorner_text,
	change_mode,
	&vs.browse_action, BA_INFO,
	'i',
	close_menu,
	};
static Flicmenu bro_vie_sel = {
	&bro_inf_sel,
	NOCHILD,
	4, 173, 44, 8,
	browse_120 /* "view" */,
	ccorner_text,
	change_mode,
	&vs.browse_action, BA_VIEW,
	'v',
	close_menu,
	};
static Flicmenu bro_loa_sel = {
	&bro_vie_sel,
	NOCHILD,
	4, 165, 44, 8,
	NOTEXT,
	ccorner_text,
	change_mode,
	&vs.browse_action, BA_LOAD,
	'l',
	close_menu,
	};
static Flicmenu bro_tit_sel = {
	&bro_loa_sel,
	NOCHILD,
	16, 156, 66, 6,
	NOTEXT,
	greytext,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	close_menu,
	};
static Flicmenu bro_tst_sel = {
	&bro_tit_sel,
	NOCHILD,
	0, 150, 319, 3,
	NOTEXT,
	grey_block,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	close_menu,
	};
static Flicmenu bro_menu = {
	NONEXT,
	&bro_tst_sel,
	0, 150, 319, 49,
	NOTEXT,
	gary_menu_back,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	close_menu,
	};

/* variables to hold x y and list position of selected file.  */

static int elx,ely,elix;

static
Name_list *
find_elix()
{
struct name_list *el;

if ((el = name_in_list(cpi_name,wild_lst)) != NULL)
	{
	elix = el_ix(wild_lst,el) - bscroller.top_name;
	if (elix >= 0 & elix < 12)	/* it's visible */
		{
		ely = brw_list_sel.y + (elix/4) * BRO_DY;
		elx = brw_list_sel.x + (elix%4) * BRO_DX;
		}
	else
		elix = -1;	/* offscreen */
	}
return(el);
}

static
which_browse()
{
elx = (uzx - brw_list_sel.x)/BRO_DX;
ely = (uzy)/BRO_DY;
elix = ely * 4 + elx;
elx *= BRO_DX;
ely *= BRO_DY;
elx += brw_list_sel.x;
}


static
bro_frame(x,y,color)
int x,y,color;
{
draw_frame(color, x, y, x+66, y+42);
}


static
draw_1_browse(x,y,el)
int x,y;
struct name_list *el;
{
char rname[16];

cblock(vf.p,x-2-4,y,71+8,49,sblack);	
bro_frame(x,y,swhite);
if (el != NULL)
	{
	strcpy(rname, el->name);
	if (rname[0] != '\\' && rname[0] != 0)
		{
		postage_stamp(x+1,y+1,el->name);
		cut_suffix(rname, ".FLI");
		}
	gtext(rname, x + (65 - CH_WIDTH*strlen(rname))/2, y+42, swhite);
	}
}

static
some_browse_line(x,y,list,something)
int x,y;
struct name_list *list;
Vector something;
{
int i;

i = 4;
while (--i >= 0)
	{
	(*something)(x,y,list);
	if (list != NULL)
		list = list->next;
	x += BRO_DX;
	}
}

static
some_browse(m,something)
Flicmenu *m;
Vector something;
{
int i, y;
struct name_list *list;

list = list_el(bscroller.name_list, bscroller.top_name);
y = m->y;
i = 3;
while (--i >= 0)
	{
	some_browse_line(m->x, y, list, something);
	y += BRO_DY;
	list = list_el(list,4);
	}
}

static
see_browse(m)
Flicmenu *m;
{
some_browse(m,draw_1_browse);
}

static
fbrowse(m)
Flicmenu *m;
{
int mx,my,mix;
long dtime;

dtime = get80hz() + 30;
which_browse();
mx = elx;
my = ely;
mix = elix;
bro_frame(mx,my,sred);
feel_1_browse(list_el(wild_lst, elix+bscroller.top_name));
bredraw_cpic();
while (dtime > get80hz())
	{
	check_input();
	which_browse();
	if (elix != mix)
		break;
	if (PJSTDN)
		{
		browse_action();
		if (vs.browse_action == 0) /* short cut to avoid redraw if load */
			return;
		}
	}
bro_frame(mx,my,swhite);
}

static
feel_browse(m)
Flicmenu *m;
{
fbrowse(m);
macrosync();
}

static
bredraw_cpic()
{
draw_sel(&bro_tna_sel);
draw_sel(&bro_cpi_sel);
}


static
new_bdrawer()
{
if (!change_dir(vs.drawer))
	return(0);
init_bscroller();
draw_sel(&bro_pat_sel);
redraw_bscroller();
return(1);
}

static
view_fli(name, screen, loop)
char *name;
Vscreen *screen;
int loop; /* 0 or 1.  1 if want to repeat animation until key hit */
{
struct fli_head fh;
int fd;
long clock;
int i;
long f1off;

if ((fd = read_fli_head(name, &fh)) == 0)
	return(0);
clock = get80hz();
mouse_on = 0;
if (loop)
	{
	if (!read_next_frame(name,fd,screen,1))
		goto OUT;
	f1off = jtell(fd);
	clock += fh.speed;
	if (!wait_til(clock))
		goto OUT;
	if (clock > get80hz())
		clock = get80hz();
	}
do 
	{
	for (i=0; i<fh.frame_count; i++)
		{
		if (!read_next_frame(name,fd,screen,1))
			goto OUT;
		clock += fh.speed;
		if (!wait_til(clock))
			goto OUT;
		if (clock > get80hz())
			clock = get80hz();
		}
	if (loop)
		jseek(fd, f1off, 0);
	}
while (loop);
OUT:
jclose(fd);
mouse_on = 1;
}


static
feel_1_browse(el)
struct name_list *el;
{
char *title;

if (el == NULL)
	return;
title = el->name;
if (title[0] == '\\')	/* directory */
	{
	if (vs.drawer[strlen(vs.drawer)-1] == '\\')  /* aviod double slash */
		title++;
	if (!dir_too_long(vs.drawer, title))
		{
		strcat(vs.drawer, title);
		new_bdrawer();
		}
	}
else if (title[0] != 0)
	{
	strcpy(vs.file, title);
	strcpy(cpi_name, title);
	if (got_suffix(vs.file))
		remove_suffix(vs.file);
	}
}

static
draw_cpi_name(m)
Flicmenu *m;
{
char buf[81];

/* deal with auto-extending BS to make up cpi_name */
strcpy(cpi_name, vs.file);
if (got_suffix(cpi_name))
	remove_suffix(cpi_name);
strcpy(buf, cpi_name);
strcat(cpi_name, ".FLI");
buf[8] = 0;	/* just make sure not too long... */
m->text = buf;
blacktext(m);
}


static
draw_cpi(m)
Flicmenu *m;
{
gary_menu_back(m);
/* if we can find it somewhere else on screen... */
if ((find_elix() != NULL) && elix >= 0)
	{
	blit8(XMAX/5,YMAX/5, elx+1, ely+1, vf.p, vf.bpr,
		m->x+1, m->y+1, vf.p, vf.bpr);
	}
else
	{
	if (jexists(cpi_name))
		{
		postage_stamp(m->x+1,m->y+1,cpi_name);
		}
	}
}


static
baction_button(m)
Flicmenu *m;
{
browse_action();
}

static
fli_info(title)
char *title;
{
struct fli_head fh;
int fd;
char buf1[40];
char buf2[40];
char buf3[40];
char *bufs[8];

if ((fd = read_fli_head(title, &fh)) == 0)
	return(0);
jclose(fd);
sprintf(buf1, browse_123 /* "%d frames in %ld bytes" */, 
	fh.frame_count, fh.size);
sprintf(buf2, browse_124 /* "Average frame size %ld" */, 
	fh.size/(fh.frame_count+1));
sprintf(buf3, browse_125 /* "Speed %d jiffies/frame" */, 
	fh.speed);
bufs[0] = title;
bufs[1] = cst_;
bufs[2] = buf1;
bufs[3] = buf2;
bufs[4] = buf3;
bufs[5] = NULL;
continu_box(bufs);
}



static
brescroll()
{
calc_scroll_pos(&bscroller, &brw_scroller_sel);
draw_sel(&brw_scroller_sel);
}

extern struct name_scroller *scroll;

static
redraw_bscroller()
{
draw_sel(&brw_list_sel);
brescroll();
}

static
bfeel_scroller(m)
Flicmenu *m;
{
fflscr(m, 0);
}


static
bincu()
{
if ((scroll->top_name -= 4) < 0)
	scroll->top_name = 0;
redraw_bscroller();
}

static
bincup(m)
Flicmenu *m;
{
hilight(m);
repeat_on_pdn(bincu);
draw_sel(m);
}

static
bincd()
{
int end;

end = scroll->name_count - scroll->ycount;
if (scroll->top_name >= end)	/* whole line past end, forget it */
	return;
scroll->top_name+= 4;
if (scroll->top_name > scroll->name_count - scroll->ycount)	/* past end... */
	{
	end_clip_scroll();
	redraw_bscroller();
	}
else
	{
	blit8(BRO_DX*4,BRO_DY*2,
		brw_list_sel.x, brw_list_sel.y+BRO_DY, vf.p, vf.bpr,
		brw_list_sel.x, brw_list_sel.y, vf.p, vf.bpr);
	some_browse_line(brw_list_sel.x, brw_list_sel.y+2*BRO_DY,
		list_el(wild_lst, scroll->top_name+8), draw_1_browse);
	brescroll();
	}
}

static
bincdown(m)
Flicmenu *m;
{
hilight(m);
repeat_on_pdn(bincd);
draw_sel(m);
}


static
init_bscroller()
{
build_wild_list();
iscroller(&bscroller, wild_lst, 
	&brw_scroller_sel, &brw_list_sel,12,redraw_bscroller);
}


static
browse_action()
{
struct name_list *el;

switch (vs.browse_action)
	{
	case BA_LOAD:	/* load */
		browse_ok = 1;
		close_menu();
		break;
	case BA_VIEW:	/* view */
		if (push_screen())
			{
			view_fli(cpi_name, &vf, 0);
			pop_screen();
			}
		else
			noroom();
		break;
	case BA_PLAY:	/* play */
		if (push_screen())
			{
			view_fli(cpi_name, &vf, 1);
			pop_screen();
			}
		else
			noroom();
		break;
	case BA_INFO:
		fli_info(cpi_name);
		break;
	case BA_KILL: /* kill */
		if (really_delete(cpi_name))
			{
			if (jdelete_rerr(cpi_name))
				{
				if ((el = find_elix()) != NULL)
					{
					el->name[0] = 0;	/* mark it dead */
					vs.file[0] = 0;		/* don't reuse name... */
					if (elix >= 0)
						draw_1_browse(elx,ely,el);
					}
				bredraw_cpic();
				}
			}
		break;
	}
}

static
scalec(c)
int c;
{
return( 64*c/6 );
}

static
make_browse_cmap()
{
int r,g,b;
UBYTE *cm;

cm = render_form->cmap;
for (r=0; r<6; r++)
	for (g=0; g<6; g++)
		for (b=0; b<6; b++)
			{
			*cm++ = scalec(r);
			*cm++ = scalec(g);
			*cm++ = scalec(b);
			}
}

static
draw_browse_menu()
{
make_browse_cmap();
see_cmap();
find_colors();
color_form(render_form,sblack);
qdraw_a_menu(&bro_menu);
}

short_path(Flicmenu *m)
/* draw clipped text */
{
int len;

len = strlen(vs.drawer);
if (len > 17)
	m->text = vs.drawer+len-17;
else
	m->text = vs.drawer;
blacktext(m);
}

/* Put up browse screen.  Return with name of file selected, or NULL if
   no file selected.  Pass in a title string and string to put on
   button for default/accept file radio button */
char  *
browse_files(say, button)
char *say, *button;
{
bro_loa_sel.text = button;
bro_tit_sel.text = say;
browse_ok = 0;	/* set our result to positive */
vs.browse_action = BA_LOAD;	/* always set to load */
hook_devices(&bro_adv_sel, 12);
redisplay_drawer = new_bdrawer;
unzoom();
push_most();
save_undo();
make_current_drawer();		/* get current directory and device... */
strcpy(wild, "*.FLI");

scroll = &bscroller;
bscroller.top_name = vs.fscroller_top-vs.fscroller_top%4;
init_bscroller();
draw_browse_menu();
nod_do_menu(&bro_menu);
vs.fscroller_top = bscroller.top_name;
free_wild_list();
unundo();
see_cmap();
pop_most();
rezoom();
return(browse_ok ? cpi_name : NULL);
}


/* do browse menu with default action to load */
go_browse()
{
char *name;

if ((name = browse_files(browse_128 /* "Browse Flics" */, 
	browse_129 /* "Load" */)) != NULL)
	{
	if (check_fli(name))
		{
		load_fli(name);
		vs.bframe_ix = 0;	/* invalidate back frame cashe */
		close_menu();
		}
	}
}

