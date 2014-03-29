/* browse.c - Stuff for browse flics.  Shares code with normal file menu
   in filemenu.c in a semi-gnarly fashion.  Wants pstamp.c for
   doing the actual read and shrink of the first frame of a fli. */

#include <string.h>
#include "jimk.h"
#include "animinfo.h"
#include "browse.h"
#include "commonst.h"
#include "errcodes.h"
#include "filepath.h"
#include "flicel.h"
#include "palmenu.h"
#include "rastcurs.h"
#include "scroller.h"
#include "softmenu.h"
#include "wildlist.h"
#include "zoom.h"

/* Browse action defines. See browse.c */
#define BA_LOAD 0
#define BA_VIEW 1
#define BA_INFO 2
#define BA_KILL 3
#define BA_PLAY 4

/* size of one browse pic/name combo */
static SHORT bro_cel_w, bro_cel_h;

/* number of browse rows and columns */
static SHORT bro_xcount, bro_ycount, bro_count;

static Names *bro_wild_list;
static char *bro_wild;

static void bredraw_cpic(void);
static int new_bdrawer(void *drawer);
static void draw_cpi(Button *m);
static void init_bscroller(int top_name);
static void browse_action(Button *m);
static void make_browse_cmap(void);

static Name_scroller bscroller;


/* Data structures for browse buttons */
static Button brw_cpi_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	66,42,102,154,
	NOTEXT,
	draw_cpi,
	NOFEEL,
	mb_close_cancel,
	NOGROUP,0,
	NOKEY,
	0	
	);
static Button brw_list_sel = MB_INIT1(
	&brw_cpi_sel,
	NOCHILD,
	299,150,20,0,
	NODATA,
	draw_scroll_cels,
	feel_scroll_cels,
	mb_close_cancel,
	&bscroller,0,
	NOKEY,
	MB_NORESCALE
	);
static Button brw_downarr = MB_INIT1(
	&brw_list_sel,
	NOCHILD,
	13,10+1,0,136,
	&ctridown,
	ccorner_image,
	scroll_incdown,
	mb_close_cancel,
	&bscroller,-1,
	DARROW,
	0
	);
static Button brw_scroller_sel = MB_INIT1(
	&brw_downarr,
	NOCHILD,
	11,123,1,12,
	&bscroller,
	see_scrollbar,
	slow_feel_scrollbar,
	mb_close_cancel,
	&bscroller,0,
	NOKEY,
	MB_NORESCALE
	);
static Button brw_uparr = MB_INIT1(
	&brw_scroller_sel,
	NOCHILD,
	13,11,0,0,
	&ctriup,
	ccorner_image,
	scroll_incup,
	mb_close_cancel,
	&bscroller,-1,
	UARROW,
	0
	);

/*** Button Data ***/

#define BRO_DRAWER  (*((char **)&(bro_pat_sel.datme)))

static Button bro_pat_sel = MB_INIT1(
	&brw_uparr,
	NOCHILD,
	124,11,195,186,
	NULL, /* set to drawer buffer */
	black_pathlabel,
	NOFEEL,
	mb_close_cancel,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button bro_tpa_sel = MB_INIT1(
	&bro_pat_sel,
	NOCHILD,
	30,7,169,188,
	NODATA, /* "Dir", */
	grey_ctext,
	NOFEEL,
	mb_close_cancel,
	NOGROUP,0,
	NOKEY,
	0
	);

static Button bro_dev_hanger = MB_INIT1(
	&bro_tpa_sel,
	NOCHILD,
	13,9,209,156,
	NODATA,
	NULL, /* set by alloc dev sels */
	NOFEEL,
	mb_close_cancel,
	NOGROUP,0,
	NOKEY,
	0
	);

static Rectangle bro_devsel_320size = {14,9,15,11}; /* width,height,dx,dy */

static Button bro_tdr_sel = MB_INIT1(
	&bro_dev_hanger,
	NOCHILD,
	30,7,175,157,
	NODATA, /* "Drive", */
	grey_ctext,
	NOFEEL,
	mb_close_cancel,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button bro_ok_sel = MB_INIT1(
	&bro_tdr_sel,
	NOCHILD,
	45,9,4,189,
	NODATA, /* "Ok", */
	ccorner_text,
	browse_action,
	mb_close_cancel,
	NOGROUP,0,
	'\r',
	0
	);

/* lots of casting need here aaack */
#define CPI_NAME (*((char **)&(bro_tna_sel.datme)))

static Button bro_tna_sel = MB_INIT1(
	&bro_ok_sel,
	NOCHILD,
	95,7,4,180,
	NULL, /* set in browse_files() */
	black_ctext,
	NOFEEL,
	mb_close_cancel,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button bro_can_sel = MB_INIT1(
	&bro_tna_sel,
	NOCHILD,
	45,9,54,189,
	NODATA, /* "Cancel", */
	ccorner_text,
	mb_close_cancel,
	mb_close_cancel,
	NOGROUP,0,
	NOKEY,
	0
	);
/* dummy entry behind cancel to add key equiv for play ... had to sneak
   it in at the last moment cause real important for presentations. */
static Button bro_play_sel = MB_INIT1(
	&bro_can_sel,
	NOCHILD,
	45,9,54,189,
	NOTEXT,
	NOSEE,
	change_mode,
	mb_close_cancel,
	&vs.browse_action,BA_VIEW,
	'v',
	MB_B_GHILITE
	);
static Button bro_del_sel = MB_INIT1(
	&bro_play_sel,
	NOCHILD,
	45,9,54,170,
	NODATA, /* "Delete", */
	ccorner_text,
	change_mode,
	mb_close_cancel,
	&vs.browse_action,BA_KILL,
	'd',
	MB_B_GHILITE
	);
static Button bro_inf_sel = MB_INIT1(
	&bro_del_sel,
	NOCHILD,
	45,9,54,162, 
	NODATA, /* "Info", */
	ccorner_text,
	change_mode,
	mb_close_cancel,
	&vs.browse_action,BA_INFO,
	'i',
	MB_B_GHILITE
	);
static Button bro_vie_sel = MB_INIT1(
	&bro_inf_sel,
	NOCHILD,
	45,9,4,170,
	NODATA, /* "Play", */
	ccorner_text,
	change_mode,
	mb_close_cancel,
	&vs.browse_action,BA_PLAY,
	'p',
	MB_B_GHILITE
	);
static Button bro_loa_sel = MB_INIT1(
	&bro_vie_sel,
	NOCHILD,
	45,9,4,162,
	NODATA, /* "Load", */
	ccorner_text,
	change_mode,
	mb_close_cancel,
	&vs.browse_action,BA_LOAD,
	'l',
	MB_B_GHILITE
	);
static Button bro_tit_sel = MB_INIT1(
	&bro_loa_sel,
	NOCHILD,
	95,7,4,153,
	NOTEXT, /* "Browse", */
	see_titlebar,
	NOFEEL,
	mb_close_cancel,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button bro_tst_sel = MB_INIT1(
	&bro_tit_sel,
	NOCHILD,
	320,4,0,147,
	NOTEXT,
	grey_block,
	NOFEEL,
	mb_close_cancel,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button brw_panel = MB_INIT1(
	NONEXT,
	&bro_tst_sel,
	320,53,0,147,
	NOTEXT,
	wbg_ncorner_back,
	NOFEEL,
	mb_close_cancel,
	NOGROUP,0,
	NOKEY,
	0
	);

static void see_browse_bg(Menuwndo *mw)
{
	make_browse_cmap();
	see_cmap();
	find_colors();
	seebg_bblack(mw);	/* seebg */
}

static Menuhdr bro_menu = {
	{320,200,0,0},	/* orig_rect */
	BROWSE_MUID,	/* id */
	PANELMENU,		/* type */
	&brw_panel,		/* buttons */
	SCREEN_FONT,	/* font */
	&menu_cursor.hdr, /* cursor */
	see_browse_bg,	/* seebg */
	NULL,			/* data */
	NULL,			/* domenu */
	(MBPEN|MBRIGHT|KEYHIT),	/* ioflags */
	0,				/* flags */
	NULL,			/* mw */
	NULL,			/* group */
	{ NULL, NULL },	/* node */
	NULL,			/* procmouse */
	NULL,			/* on_showhide */
	NULL,			/* cleanup */
	0, 0, 0, 0		/* scaled width, height, x, y */
};

static char *cant_read_str;

static Smu_button_list bro_smblist[] = {
#define BRO_TITLE_KEY bro_smblist[0].name
	{ NULL,     { /* butn */ &bro_tit_sel } },
#define BRO_LOADB_KEY bro_smblist[1].name
	{ NULL,     { /* butn */ &bro_loa_sel } },
	{ "dir",    { /* butn */ &bro_tpa_sel } },
	{ "drive",  { /* butn */ &bro_tdr_sel } },
	{ "ok",     { /* butn */ &bro_ok_sel } },
	{ "cancel", { /* butn */ &bro_can_sel } },
	{ "del",    { /* butn */ &bro_del_sel } },
	{ "info",   { /* butn */ &bro_inf_sel } },
	{ "play",   { /* butn */ &bro_vie_sel } },

	/* texts note first char is a 'T' */
	{ "Tcant_read", { /* ps */ &cant_read_str } },
};

/* variables to hold x y and list position of selected file.  */

static int find_elix(Short_xy *spos, Names **pel)
/* returns -2 if element not found -1 if not in scroller 0 and up is index of
 * a visible cel */
{
int elix;

	if((*pel = name_in_list(CPI_NAME,bro_wild_list)) != NULL)
	{
		elix = slist_ix(bro_wild_list, *pel) - bscroller.top_name;
		if (elix >= 0 & elix < bro_count)	/* it's visible */
		{
			get_scroll_cel_pos(&bscroller, elix, spos);
			return(elix);
		}
		else
			return(-1); /* offscreen */
	}
	return(-2);
}

static void bro_frame(Raster *r, Rectangle *rect, int color)
{
	draw_quad(r,color,rect->x, rect->y, rect->width, rect->height );
}

static void find_el_rect(Raster *r,Rectangle *rect, int elx, int ely)

/* Quite a fudge, but the actual rectangle isnt stored anywhere so
 * we read it from the screen itself. Since, hopefully, it is there. */
{
Coor doty, dotx;

	rect->x = elx;
	rect->y = ely;
	rect->width = elx + brw_cpi_sel.width - 1;
	rect->height = ely + brw_cpi_sel.height - 1;

	doty = ely + brw_cpi_sel.height/2;
	dotx = elx + brw_cpi_sel.width/2;

	while(pj_get_dot(r,rect->x,doty) == sblack)
		++rect->x;
	while(pj_get_dot(r,dotx,rect->y) == sblack)
		++rect->y;
	while(pj_get_dot(r,rect->x,rect->height) == sblack)
		--rect->height;
	while(pj_get_dot(r,rect->width,rect->y) == sblack)
		--rect->width;
	rect->width -= (rect->x - 1);
	rect->height -= (rect->y - 1);
}
static void
text_1_browse(Raster *rast, Vfont *font, Names *el, Pixel color, int x, int y)
{
int textx;

	textx = x + font_xcent_oset(font,el->name,brw_cpi_sel.width);
	if(textx < x)
		textx = x;

	gftext(rast, font, el->name, textx,
		y+brw_cpi_sel.height+1+vb.screen->is_hires, 
		color, TM_MASK1);
}
static void
hilite_1_browse(Button *b, Raster *rast, int x, int y,
		Names *entry, Boolean hilit)
{
Rectangle brect;
Pixel color = hilit?sred:swhite;

	find_el_rect(rast,&brect,x,y);
	text_1_browse(rast,b->root->font,entry,color,x,y);
	bro_frame(rast,&brect,color);
}
static void draw_1_browse(Button *b, Raster *rast, int x, int y, Names *el)
{
Errcode err;
Name_scroller *scr = b->group;
Vfont *font = b->root->font;
Rectangle psize;

	pj_set_rect(rast,sblack,x,y,scr->cel_width,scr->cel_height);	
	psize.x = x;
	psize.y = y;
	psize.width = brw_cpi_sel.width;
	psize.height = brw_cpi_sel.height;
	if (el != NULL)
	{
		if (el->name[0] != DIR_DELIM && el->name[0] != 0)
		{
			if((err = postage_stamp(rast,el->name,x+1,y+1,brw_cpi_sel.width-2,
						brw_cpi_sel.height-2,&psize)) < Success)
			{
				gftext(rast, font, cant_read_str,
						psize.x + font_xcent_oset(font,cant_read_str,
												  psize.width),
						psize.y + font_ycent_oset(font,psize.height),
						sred, TM_MASK1);
			}
			else
			{
				psize.x -= 1;
				psize.y -= 1;
				psize.width += 2;
				psize.height += 2;
			}
		}
		text_1_browse(rast,font,el,swhite,x,y);
	}
	bro_frame(rast,&psize,swhite);
}
static void
feel_1_browse(Button *list_sel, Raster *rast, int x, int y,
		Names *entry, int why)
{
char *title;
(void)rast;
(void)x;
(void)y;

	title = entry->name;
	if(title[0] == DIR_DELIM)	/* directory */
	{
		if(why == SCR_ARROW)
			return;
		add_subpath(BRO_DRAWER, &title[1], BRO_DRAWER);
		strcat(BRO_DRAWER,DIR_DELIM_STR);
		new_bdrawer(BRO_DRAWER);
	}
	else if(title[0] != 0)
		strcpy(CPI_NAME, title);

	bredraw_cpic();

	if(why & (SCR_MDHIT|SCR_ENTER))
		browse_action(list_sel);
}

static void bredraw_cpic(void)
{
	draw_buttontop(&brw_cpi_sel); /* this will cancel cpi_name if not found */
	draw_buttontop(&bro_tna_sel);
}

static int new_bdrawer(void *drawer)
{
Errcode err;

	if((err = change_dir(drawer)) < Success)
		return(err);
	init_bscroller(0);
	draw_buttontop(&bro_pat_sel);
	draw_button(&bro_dev_hanger);
	redraw_scroller(&bscroller);
	return(0);
}

static Errcode view_anim(char *name,
						 Rcel *screen,
					     Boolean loop) 
/* View an animation or cel */
/* loop is 0 or 1.  1 if want to repeat animation until key hit */
{
Errcode err;
Flifile flif;
LONG clock;
Rcel stack_cel;	/* (clipped) cel to play fli */
Rcel *cel;
int i;

	if((err = pj_fli_open(name, &flif, JREADONLY)) < Success)
	{
		pj_clear_rast(screen);
		if((err = load_pic(name,screen,0,TRUE)) >= Success)
		{
			if(loop)
				wait_click();
			else
				timed_wait_input(ANY_CLICK, 5000); /* wait 5 seconds */
		}
		return(err);
	}

	cel = center_virtual_rcel(screen, &stack_cel, 
							  flif.hdr.width, flif.hdr.height);
	if (cel->width < screen->width || cel->height < screen->height)
		pj_clear_rast(screen);

	pj_seek(flif.fd, flif.hdr.frame1_oset, JSEEK_START);
	clock = pj_clock_1000();
	hide_mouse();

	i = 0;
	do 
	{
		for (; i<=flif.hdr.frame_count; i++)
		{
			if((pj_fli_read_next(name,&flif,cel,1)) < 0)
				goto OUT;
			clock += flif.hdr.speed;
			if (!wait_til(clock))
				goto OUT;
			if (clock > pj_clock_1000())
				clock = pj_clock_1000();
		}
		if (loop)
		{
			pj_seek(flif.fd, flif.hdr.frame2_oset, JSEEK_START);
			i = 1; /* got first frame already last ringed it */
		}
	}
	while (loop);

OUT:
	pj_fli_close(&flif);
	show_mouse();
	return(err);
}


static void draw_cpi(Button *m)
{
Raster *root = (Raster *)(m->root);
Short_xy spos;
Names *el;
int elix;
Rectangle brect;

	white_block(m);

	/* if we can find it somewhere else on screen... */
	if((elix = find_elix(&spos,&el)) >= 0)
	{
		find_el_rect(root,&brect, spos.x, spos.y);

		/* we get the border with this but it will be over written below */
		pj_blitrect(root, brect.x, brect.y, 
				    root, m->x + brect.x - spos.x, m->y + brect.y - spos.y, 
					brect.width, brect.height);
		brect.x += (m->x - spos.x);
		brect.y += (m->y - spos.y);
	}
	else /* see if there is a file */
	{
		if (!pj_exists(CPI_NAME))
			return;
	 
		if((postage_stamp(root,CPI_NAME,m->x+1,m->y+1,
			  		      brw_cpi_sel.width-2, brw_cpi_sel.height-2,&brect)) 
					         < Success)
		{
			return;		
		}

		brect.width += 2;
		brect.height += 2;
		--brect.x;
		--brect.y;
	}
	bro_frame(root,&brect,sgrey);
}


static Errcode show_fli_info(char *title)
{
Errcode err;
Fli_head fh;
Jfile fd = JNONE;
Pic_header pic;
LONG size;


	if((err = pj_fli_read_head(title, &fh, &fd, JREADONLY)) >= Success)
	{
		soft_continu_box( "!%s%d%ld%ld%d%d%d", "browse_info",
						  title, fh.frame_count, fh.size,
						  fh.size/(fh.frame_count+1),
						  millisec_to_jiffies(fh.speed), fh.width, fh.height );
	}
	else
	{
		if ((fd = pj_open(title, JREADONLY)) == JNONE)
		{
			err = pj_ioerr();
			goto error;
		}
		if(pj_read_pichead(fd,&pic) < Success) /* note use error code above */ 
			goto error;

  		if((size = pj_seek(fd,0,JSEEK_END)) < 0)
		{
			err = fh.size;
			goto error;
		}

		soft_continu_box( "!%s%ld%d%d", "browse_cel_info",
						  title, size, pic.width, pic.height );
	}
	err = Success;
error:
	pj_close(fd);
	return(softerr(err,"!%s","bro_info",title));
}

static void init_bscroller(int top_name)
{
char *wilds;
char wild[WILD_SIZE];
Names *wlist;
Boolean get_dirs = TRUE;

	free_wild_list(&bro_wild_list);

	wilds = bro_wild;
	while(parse_to_semi(&wilds, wild,sizeof(wild)))
	{
		if((build_wild_list(&wlist,wild,get_dirs)) < Success)
			continue;
		get_dirs = FALSE;
		bro_wild_list = merge_wild_lists(wlist,bro_wild_list);
	}

	clear_struct(&bscroller);
	bscroller.top_name = top_name;
	bscroller.names = bro_wild_list;
	bscroller.scroll_sel = &brw_scroller_sel;
	bscroller.list_sel = &brw_list_sel;

	bscroller.draw_1_cel = draw_1_browse;
	bscroller.feel_1_cel = feel_1_browse;
	bscroller.high_1_cel = hilite_1_browse;

	bscroller.font = vb.screen->mufont;
	bscroller.cels_per_row = bro_xcount;
	bscroller.cels_per_col = bro_ycount;
	bscroller.no_key_mode = TRUE;
	init_scroller(&bscroller,vb.screen);
}

static void save_view_anim(Boolean loop)
/* save current screen and view fli */
{
Errcode err;
Rcel_save opic;

	if (report_temp_save_rcel(&opic, vb.cel_a) >= Success)
	{
		err = view_anim(CPI_NAME, vb.cel_a, loop);
		report_temp_restore_rcel(&opic, vb.cel_a);
	}
	else
	{
		err = view_anim(CPI_NAME, vb.cel_a, loop);
		make_browse_cmap();
		find_colors();
		draw_menu(&bro_menu);
	}
	see_cmap();
	softerr(err, "!%s", "bro_view", CPI_NAME);
}


static void browse_action(Button *m)
{
char fpath[PATH_SIZE];
Names *el;
int elix;
Short_xy spos;

	if(CPI_NAME[0] == 0)
		return;

	full_path_name(BRO_DRAWER, CPI_NAME, fpath);

	switch (vs.browse_action)
	{
		case BA_LOAD:	/* load */
			mb_close_ok(m);
			return;
		case BA_VIEW:	/* view */
			save_view_anim(FALSE);
			return;
		case BA_PLAY:	/* play */
			save_view_anim(TRUE);
			return;
	}


	switch(vs.browse_action)
	{
		case BA_INFO:
			show_fli_info(fpath);
			break;
		case BA_KILL: /* kill */
		{
			if(really_delete(fpath))
			{
				if((softerr(pj_delete(fpath),
							"!%s","cant_delete", fpath )) < Success)
				{
					break;
				}
				if((elix = find_elix(&spos,&el)) >= -1)
				{
					el->name[0] = 0;	/* mark it dead */
					if(elix >= 0)
					{
						draw_1_browse(&brw_list_sel,
									  (Raster *)(m->root),spos.x,spos.y,el);
					}
				}
				CPI_NAME[0] = 0;
				bredraw_cpic();
			}
			break;
		}
	}
}


static void make_browse_cmap(void)
{
int r, g, b;
UBYTE *cm;

	cm = (UBYTE *)(vb.pencel->cmap->ctab);
	for (r=0; r<6; r++)
	{
		for (g=0; g<6; g++)
		{
			for (b=0; b<6; b++)
			{
				*cm++ = RGB_MAX*r/6;
				*cm++ = RGB_MAX*g/6;
				*cm++ = RGB_MAX*b/6;
			}
		}
	}
}

static void format_browse_menu(void)
/* Figure out the dimensions of browse buttons */
{
SHORT sw,sh;
SHORT dx,dy;
SHORT pixbor;		/* single pixel in lo res, 2 in hi */
SHORT pixbor2;		/* 2*pixbor */
SHORT listw, listh;
SHORT xrema;

pixbor = 1 + vb.screen->is_hires;
pixbor2 = pixbor+pixbor;

/* rescale buttons to the screen and then disabel further scaling so values
 * can be adjusted here */

scale_button_list(bro_menu.mbs,&vb.screen->menu_scale);

bro_menu.flags |= (MENU_NORESCALE|MENU_NOMB_RESCALE);

/* make basic menu full screen and set not to be scaled */
bro_menu.width = sw = vb.screen->wndo.width;
bro_menu.height = sh = vb.screen->wndo.height;

/* put panel part on the bottom and centered */
dy = sh - (brw_panel.y + brw_panel.height);
dx = sw - (brw_panel.x + brw_panel.width);
dx >>= 1;
offset_button_list(&brw_panel, dx, dy);	

/* put scroll bar and arrows flush to left and scaled to take up all of
   screen not part of panel */
brw_uparr.x = brw_uparr.y = brw_downarr.x = brw_list_sel.y = 0;
brw_downarr.y = brw_panel.y - brw_downarr.height - pixbor;
brw_scroller_sel.x = 1;
brw_scroller_sel.y = brw_uparr.y + brw_uparr.height + 1;
brw_scroller_sel.height = brw_downarr.y - brw_scroller_sel.y - 1;
brw_scroller_sel.width = brw_downarr.width - 2;

/* calculate dimensions of a single browse box including image & name */
/* brw_cpi_sel is our starting point (the image contained in the panel) */
dx = brw_cpi_sel.width + pixbor2;	/* minimum cel size */
dy = brw_cpi_sel.height + tallest_char(vb.screen->mufont);
listw = sw - brw_uparr.width - pixbor2 - brw_cpi_sel.width;
listh = brw_panel.y;
bro_xcount = listw/dx;
bro_ycount = listh/dy;
bro_cel_w = listw/bro_xcount;
bro_cel_h = listh/bro_ycount;
xrema = listw - bro_cel_w*bro_xcount;

bro_xcount += 1;		/* now can include the flush to the right last one */
bro_count = bro_xcount*bro_ycount;
brw_list_sel.width = listw + brw_cpi_sel.width - xrema;
brw_list_sel.x = sw-brw_list_sel.width;
brw_list_sel.height = listh;
}

static Errcode
browse_anims(char *inpath, char *outpath, char *title_key, char *loadb_key,
		SHORT *scroll_top)
/* Put up browse screen.  Return with name of file selected, or NULL if
   no file selected.  Pass in a title string and string to put on
   button for default/accept file radio button */
{
Errcode err;
char odir[PATH_SIZE];
char drawer[PATH_SIZE];
char name[PATH_SIZE];
void *ss = NULL;
static char panel_key[] = "browse_panel";

	split_copy_path(inpath, drawer, name);
	get_dir(odir);
	if ((err = change_dir(drawer)) < Success)
		softerr(err, "!%s", "cant_find", drawer);
	make_good_dir(drawer);

	BRO_DRAWER = drawer;
	CPI_NAME = name;
	BRO_LOADB_KEY = loadb_key;
	BRO_TITLE_KEY = title_key;
	vs.browse_action = BA_LOAD;	/* always set to load */

	unzoom();
	save_undo();

	if((err = soft_buttons(panel_key, bro_smblist, Array_els(bro_smblist),
							&ss )) < Success)
	{
		goto error;
	}
	if((err = alloc_dev_sels(&bro_dev_hanger,&bro_devsel_320size,7,3,
							 drawer, new_bdrawer, drawer )) < Success)
	{
		goto error;
	}

	format_browse_menu();
	bro_wild = "*.FLC;*.FLI;*.CEL";
	init_bscroller(*scroll_top - (*scroll_top%bro_xcount));
	err = do_reqloop(vb.screen,&bro_menu,NULL,NULL,NULL);
	*scroll_top = bscroller.top_name;

error:
	smu_free_scatters(&ss);
	free_wild_list(&bro_wild_list);
	cleanup_dev_sels(&bro_dev_hanger);
	zoom_unundo();
	see_cmap();
	rezoom();
	full_path_name(drawer, name, outpath);
	change_dir(odir);
	return(softerr(err,"bro_menu"));
}

void go_browse(void)
/* do browse menu with default action to load */
{
char name[PATH_SIZE];
Vset_path cpath;

	vset_get_pathinfo(FLI_PATH, &cpath);
	if(browse_anims(cpath.path, name, "fli_title", 
					"load", &cpath.scroller_top) >= Success)
	{
		if (confirm_dirty_load())
			resize_load_fli(name);
		strcpy(cpath.path,name);
	}
	vset_set_pathinfo(FLI_PATH,&cpath);
}

Errcode go_browse_cels(void)
/* do browse menu with default action to load */
{
Errcode err;
char name[PATH_SIZE];
Vset_path cpath;

	vset_get_pathinfo(CEL_PATH, &cpath);
	if ((err = browse_anims(cpath.path, name, "cel_title",
					"load", &cpath.scroller_top)) >= Success)
	{
		err = load_the_cel(name);
		strcpy(cpath.path,name);
	}
	vset_set_pathinfo(CEL_PATH,&cpath);
	return err;
}
