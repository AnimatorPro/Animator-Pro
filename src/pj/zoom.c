/* zoom.c - help simulate fat-bits.  Position zoom window on screen. */

#include "jimk.h"
#include "errcodes.h"
#include "ftextf.h"
#include "grid.h"
#include "marqi.h"
#include "memory.h"
#include "menus.h"
#include "pentools.h"
#include "ptrmacro.h"
#include "rastcurs.h"
#include "rastlib.h"
#include "softmenu.h"
#include "zoom.h"

static char esize_zwin_line[] = "zoom_win";

static void see_zoom_title(Button *b);
static void see_zoombox(Button *b);
static void feel_zoombox(Button *b);
static void move_zwinmenu(Button *b, void *data);
static void qset_zoom_source(void);
static void reposit_zoomwndo(void);
static void hmp_zwin_repos(void);
static void see_fullsize(Button *b);
static void zwin_fullsize(Button *b);
static void feel_scale_slider(Button *b);
static void deltascale_zwndo(void *data, Button *b);

#define ZBOX_BORSIZE	2

static Button zoom_source_sel = MB_INIT1(
	NONEXT,					/* next */
	NOCHILD,           		/* children */
	54,11,63,26,       		/* width height x y */
	NODATA, /* "Source",*/
	ccorner_text,			/* seeme */
	qset_zoom_source,		/* feelme */
	NOOPT,					/* optme */
	NOGROUP,				/* group */
	0,						/* id */
	NOKEY,					/* key equivalent */
	0						/* flags */
	);
static Button zoom_resize_sel = MB_INIT1(
	&zoom_source_sel,					/* next */
	NOCHILD,           		/* children */
	54,11,63,12,       		/* width height x y */
	NODATA, /*"Dest",*/
	ccorner_text,			/* seeme */
	hmp_zwin_repos,		/* feelme */
	NOOPT,					/* optme */
	NOGROUP,				/* group */
	0,						/* id */
	NOKEY,					/* key equivalent */
	0						/* flags */
	);
static Button zoom_fullsize_sel = MB_INIT1(
	&zoom_resize_sel,		/* next */
	NOCHILD,           		/* children */
	54,11,5,12,        		/* width height x y */
	NODATA, /* "Fullsize",*/
	see_fullsize,
	zwin_fullsize,			/* feelme */
	NOOPT,					/* optme */
	NOGROUP,				/* group */
	0,						/* id */
	NOKEY,					/* key equivalent */
	0						/* flags */
	);

static Qslider zscale_qslider = 
	QSL_INIT1( 2, 16, &vs.zoomscale,0,deltascale_zwndo,leftright_arrs ); 

static Button zoom_scale_slider = MB_INIT1(
	&zoom_fullsize_sel,		/* next */
	NOCHILD,           		/* children */
	113,11,5,40,       		/* width height x y */
	&zscale_qslider,		/* datme */
	see_qslider,			/* seeme */
	feel_scale_slider,		/* feelme */
	NOOPT,					/* optme */
	NOGROUP,				/* group */
	0,						/* id */
	NOKEY,					/* key equivalent */
	0						/* flags */
	);

static Button zoom_scaletext = MB_INIT1(
	&zoom_scale_slider,		/* next */
	NOCHILD,           		/* children */
	122,8,0,51,        		/* width height x y */
	NODATA, /* "SCALE",*/
	grey_ctext,				/* seeme */
	NULL,					/* feelme */
	NOOPT,					/* optme */
	NOGROUP,				/* group */
	0,						/* id */
	NOKEY,					/* key equivalent */
	0						/* flags */
	);

static Button zoom_tit_sel = MB_INIT1(
	&zoom_scaletext,		/* next */
	NOCHILD,           		/* children */
	122,9,0,0,       		/* width height x y */
	NODATA, /* "Zoom Settings",*/
	see_titlebar,			/* seeme */
	feel_titlebar,			/* feelme */
	mb_menu_to_bottom,		/* optme */
	&tbg_moveclose,			/* group */
	0,						/* id */
	'q',					/* key equivalent */
	0						/* flags */
	);


static Menuhdr zoom_menu = {
	{122,62,0,0},   /* width, height, x, y */
	ZOOM_MUID,		/* id */
	PANELMENU,		/* type */
	&zoom_tit_sel,	/* buttons */
	SCREEN_FONT,	/* font */
	&menu_cursor.hdr, /* cursor */
	seebg_white, 	/* seebg */
	NULL,			/* dodata */
	NULL,			/* domenu */
	(MBPEN|MBRIGHT|KEYHIT), /* ioflags */
	0,				/* flags */
	NULL,			/* mw */
	NULL,			/* group */
	{ NULL, NULL },	/* node */
	NULL,			/* procmouse */
	NULL,			/* on_showhide */
	NULL,			/* cleanup */
	0, 0, 0, 0		/* scaled width, height, x, y */
};

static void titbar_close_zwinmenu(Button *b, void *data)
{
	(void)b;
	(void)data;

	close_zwinmenu(); /* closeit */
	draw_buttontop(&zpan_cycle_group);
}

static Titbar_group z_tbg = {
 	move_zwinmenu,	/* moveit */
	titbar_close_zwinmenu, /* closeit */
	NULL /* data */
};


static Button zoom_dragbar = MB_INIT1(
	NONEXT,  	/* next */
	NOCHILD,    /* children */
	320,8,0,0,	/* w,h,x,y */
	NOTEXT,
	see_zoom_title,	/* seeme */
	feel_titlebar, /* feelme */
	reposit_zoomwndo, /* optme */
	(void *)&z_tbg,    /* group */
	0,			/* identity */
	NOKEY,      /* key equivalent */
	MB_NOHILITE|MB_NORESCALE /* flags */
);

static Button zoom_box = MB_INIT1(
	&zoom_dragbar,  /* next */
	NOCHILD,    /* children */
	0,0,0,0,	/* w,h,x,y */
	NULL,       /* datme */
	see_zoombox,	/* seeme */
	feel_zoombox,	/* feelme */
	NOOPT,      /* optme */
	NOGROUP,    /* group */
	0,			/* identity */
	NOKEY,      /* key equivalent */
	MB_NORESCALE	/* flags */
);

static Menuhdr zwinmenu = {
	{ 0,0,0,0 }, 	/* rect calculated by open_zwinmenu */
	ZOOM_WNDOID,	/* id */
	PANELMENU,		/* type */
	&zoom_box,		/* buttons */
	SCREEN_FONT,	/* font */
	&zoom_pencel_cursor,	/* cursor */
	seebg_none,		/* seebg */
	NODATA,         /* dodata */
	do_menubuttons,	/* domenu */
	(MBPEN|MBRIGHT), /* ioflags */
	0,				/* flags */
	NULL,			/* mw */
	NULL,			/* group */
	{ NULL, NULL },	/* node */
	NULL,			/* procmouse */
	NULL,			/* on_showhide */
	NULL,			/* cleanup */
	0, 0, 0, 0		/* scaled width, height, x, y */
};

Boolean check_zoom_drag(void)
/* this is a real total kludge to enable the text editor to drag and close
 * the zoom window title bar drags etc should be part of input system */
{
	if( (!JSTHIT(MBPEN|MBRIGHT))
		|| !vs.zoom_open 
		|| icb.iowndo != &(zwinmenu.mw->w)
		|| !cursin_menu(&zwinmenu))
	{
		return(FALSE);
	}
	return(button_keyhit(&zwinmenu,&zoom_dragbar,NULL));
}

static void scale_zwinmenu(void)
{
	/* we do need to scale some fields of the dragbar but not all of them.
	 * The rest are all calculated */

	zoom_dragbar.height = rscale_y(&vb.screen->menu_scale,8);
	zwinmenu.flags |= MENU_NORESCALE;
}


static void see_zoom_title(Button *b)
{
char buff[40];
void *odat;

	odat = b->datme;
	snftextf(buff,sizeof(buff),"!%d", b->datme, vs.zoomscale );
	b->datme = buff;
	see_titlebar(b);
	b->datme = odat;
}
static void see_zoombox(Button *b)
{
	if(!(b->y)) /* no border */
		return;

	draw_quad((Raster *)b->root, vb.screen->SWHITE, b->x-1, b->y-1,
			  b->width + 2, b->height +2);
	draw_quad((Raster *)b->root, vb.screen->SGREY, b->x-2, b->y-2,
			  b->width + 4, b->height + 4);
}
static void feel_zoombox(Button *b)
{
	(void)b;

	if((PENWNDO->doit)
		&& ISINPUT(PENWNDO->ioflags))
	{
		(*(PENWNDO->doit))(vl.zoomwndo);
	}
}
static void save_zwinpos(void)
{
	vl.zwincent.x = zwinmenu.x + (zwinmenu.width>>1);
	vl.zwincent.y = zwinmenu.y + (zwinmenu.height>>1);
	vs.zwincentx = scale_vscoor(vl.zwincent.x, vb.screen->wndo.width);
	vs.zwincenty = scale_vscoor(vl.zwincent.y, vb.screen->wndo.height);
	vl.zwinw = zwinmenu.width;
	vl.zwinh = zwinmenu.height;
	vs.zwinw = scale_vscoor(vl.zwinw, vb.screen->wndo.width);
	vs.zwinh = scale_vscoor(vl.zwinh, vb.screen->wndo.height);
}
static void save_zrectpos(void)
/* moves current zrect center to the vs.zcent as a scaled field */
{
	vs.zcentx = scale_vscoor(vl.zrect.x + vl.zrect.width/2,
							 vb.pencel->width);
	vs.zcenty = scale_vscoor(vl.zrect.y + vl.zrect.height/2,
							 vb.pencel->height);
}
static void move_zwinmenu(Button *b, void *data)
{
	(void)data;

	mb_move_menu(b);
	save_zwinpos();
}

Boolean curs_in_zoombox(void)
{
	return(ptin_rect((Rectangle *)&(zoom_box.RECTSTART),
					  icb.sx - vl.zoomwndo->behind.x,
					  icb.sy - vl.zoomwndo->behind.y));
}



void get_zoomcurs_flixy(Short_xy *xy)
{
	/* get fli window pixel position from screen cursor postition */

	xy->x = (icb.cx - (vl.zoomwndo->behind.x + zoom_box.x))/vs.zoomscale
					+ vl.zrect.x;
	xy->y = (icb.cy - (vl.zoomwndo->behind.y + zoom_box.y))/vs.zoomscale
					+ vl.zrect.y;
}
static void proc_zoomouse(void)
/* we make icb.mx relative to the pencel NOT the zoom wndo 
 * and put cursor at fat pixel screen position */
{

	icb.mx = ((icb.mx - zoom_box.x)/vs.zoomscale) + vl.zrect.x;
	icb.my = ((icb.my - zoom_box.y)/vs.zoomscale) + vl.zrect.y;

	if(vs.use_grid)
		grid_flixy(&icb.mx,&icb.my);

	if(curs_in_zoombox())
	{
		/* invert above operation to get back to screen */

		icb.cx = ((icb.mx - vl.zrect.x) * vs.zoomscale) + zoom_box.x 
				+ icb.mset.oset.x;
		icb.cy = ((icb.my - vl.zrect.y) * vs.zoomscale) + zoom_box.y 
				+ icb.mset.oset.y;
		set_wndo_cursor(vl.zoomwndo, &zoom_pencel_cursor);
	}
	else
		set_wndo_cursor(vl.zoomwndo, &menu_cursor.hdr);

	if(icb.cx == icb.lastcx && icb.cy == icb.lastcy) /* cancel mouse move */
		icb.state &= ~(MMOVE);
}
static int clip_zwinrect(Rectangle *wrect)

/* clips zwinmenu to screen and fli window zoomed size and (opt) borders
 * returns 1 if full screen and no borders were added 0 if bordered and
 * zoom box is smaller than screen */
{
SHORT zwid, zht, mindim;

	scale_zwinmenu();
	zwid = (vb.pencel->width * vs.zoomscale);
	zht = (vb.pencel->height * vs.zoomscale);
	
	if(zwid >= vb.screen->wndo.width
	    && zht >= vb.screen->wndo.height
		&& wrect->width >= vb.screen->wndo.width
		&& wrect->height >= vb.screen->wndo.height)
	{
		copy_rectfields(&(vb.screen->wndo),wrect);
		return(1);
	}

	if(zwid > vb.screen->wndo.width) /* zoom box no bigger than screen */
		zwid = vb.screen->wndo.width;
	if(zht > vb.screen->wndo.height)
		zht = vb.screen->wndo.height;

	if(wrect->width < (mindim = zoom_dragbar.height+ZBOX_BORSIZE*3))
		wrect->width = mindim;
	else if(wrect->width > (zwid += ZBOX_BORSIZE*2))
		wrect->width = zwid;

	if(wrect->height < mindim)
		wrect->height = mindim;
	else if(wrect->height > (zht += (zoom_dragbar.height + ZBOX_BORSIZE)))
		wrect->height = zht;

	return(0);
}
static void recenter_zwinrect(void)
{
	zwinmenu.x = vl.zwincent.x - (zwinmenu.width>>1);
	zwinmenu.y = vl.zwincent.y - (zwinmenu.height>>1);
}

static void get_zwinrect(void)
/* loads stored window position into zwinmenu */
{
	zwinmenu.width = vl.zwinw;
	zwinmenu.height = vl.zwinh;
	if(!clip_zwinrect((Rectangle *)&(zwinmenu.RECTSTART)))
		recenter_zwinrect();
}
static void size_zrect(Rectangle *zrect, SHORT w,SHORT h)

/* calculates zrect to and clips zoomwindow size for current zoomscale */
{
	zrect->width = (w+vs.zoomscale-1)/vs.zoomscale;		/* round up if */
	zrect->height = (h+vs.zoomscale-1)/vs.zoomscale;	/* xx%zoomscale != 0 */

	/* move to saved settings center position */

	zrect->x = uscale_vscoor(vs.zcentx,vb.pencel->width) 
						- zrect->width/2;
	zrect->y = uscale_vscoor(vs.zcenty,vb.pencel->height) 
						- zrect->height/2;

	/* clip to fli window */

	bclip0xy_rect(zrect,(Rectangle *)&(vb.pencel->RECTSTART));
}

static void calc_zoombox(void)
/* Calculates sizes of zoom rectangles and window sizes clipped to the
 * current environment.  Must be called before zoom menu is opened and 
 * not while it is open */
{
int borderless;

	borderless = clip_zwinrect((Rectangle *)&(zwinmenu.RECTSTART));
	zoom_box.width = zwinmenu.width;
	zoom_box.height = zwinmenu.height;

	if(borderless)
	{
		/* full screen zoom window */
		zoom_box.x = zoom_box.y = 0;
		zoom_box.next = NULL;
	}
	else
	{
		zoom_box.x = ZBOX_BORSIZE;
		zoom_box.y = zoom_dragbar.height; 
		zoom_dragbar.width = zoom_box.width;
		zoom_box.width -= zoom_box.x+ZBOX_BORSIZE;
		zoom_box.height -= zoom_box.y+ZBOX_BORSIZE;
		zoom_box.next = &zoom_dragbar;
	}
	size_zrect(&vl.zrect,zoom_box.width,zoom_box.height);
}

static void cleanup_zwinmenu(Menuhdr *mh)
{
	(void)mh;
	vl.zoomwndo = NULL;
}
void close_zwinmenu(void)
{
	close_menu(&zwinmenu);
	vs.zoom_open = 0;
	draw_buttontop(&zoom_fullsize_sel);
	smu_free_text((char **)&zoom_dragbar.datme);
}

static Errcode open_zwinmenu(int squawk)
{
Errcode err;
Rectangle maxrect;

	err = smu_load_name_text(&smu_sm, "zoom_panel", "Tzwin_title",
			(char **)&zoom_dragbar.datme);
	if (err < Success)
	{
		goto error;
	}
	zwinmenu.cleanup = cleanup_zwinmenu;
	zwinmenu.procmouse = proc_zoomouse;
	if(!(zwinmenu.flags & MENU_NOBCLIP))
	{
		copy_rectfields(&vb.screen->wndo,&maxrect);
		maxrect.x -= ZBOX_BORSIZE;
		maxrect.width += ZBOX_BORSIZE*2;
		maxrect.height += ZBOX_BORSIZE + zoom_dragbar.height;
		bclip_rect((Rectangle *)&zwinmenu.RECTSTART,&maxrect);
		zwinmenu.flags |= MENU_NOBCLIP; /* only clip first time user starts */
	}
	if((err = open_menu(vb.screen, &zwinmenu, NULL, PENWNDO)) < 0)
		goto error;

	/* update vs for new position if clipped etc */

	save_zwinpos();
	vl.zoomwndo = &(zwinmenu.mw->w);
	vs.zoom_open = 1;
	zoom_it();
	return(0);
error:
	if(squawk)
		err = softerr(err,"zoom_open");
	close_zwinmenu();
	return(err);
}

static Errcode calc_open_zwinmenu(int squawk)
{
Errcode err;

	calc_zoombox();
	err = open_zwinmenu(squawk);
	draw_buttontop(&zoom_fullsize_sel);
	return(err);
}
static Errcode scroll_zoomwndo(void)

/* called on a pen down, will scroll the zoom window viewport (vl.zrect)
 * over the pencel */
{
Rectangle orect;
SHORT lastx, lasty, firstsx, firstsy;
SHORT scale;

	orect = vl.zrect;
	firstsx = icb.sx;
	firstsy = icb.sy;
	scale = vs.zoomscale;
	if (scale > 4)
		scale = 4;

	for (;;)
	{
		wait_input(MMOVE|ANY_CLICK);
		if(JSTHIT(MBPEN))
			break;
		if (JSTHIT(MBRIGHT|KEYHIT))
		{
			vl.zrect = orect;
			zoom_it();
			break;
		}
		lastx = vl.zrect.x;
		lasty = vl.zrect.y;
		vl.zrect.x = orect.x - ((scale * (icb.sx - firstsx))/vs.zoomscale);
		vl.zrect.y = orect.y - ((scale * (icb.sy - firstsy))/vs.zoomscale);

		bclip0xy_rect(&vl.zrect,(Rectangle *)&(vb.pencel->RECTSTART));

		if(vl.zrect.x != lastx || vl.zrect.y != lasty)
			zoom_it();
	}
	save_zrectpos();
	return(0);
}
Errcode zoom_handtool(void)

/* the one called for the pan tool when mouse is over zoom window */
{
	if(zoom_box.next != NULL && button_keyhit(&zwinmenu,&zoom_dragbar,NULL))
		return(0);
	if(JSTHIT(MBRIGHT|KEYHIT))
		return(Err_abort);
	return(scroll_zoomwndo());
}

static void qset_zoom_source(void)
/* have user rubberband rectangle around area he/she wants zoomed */
{
Errcode err;
Rectangle rect;

	hide_mp();
	close_zwinmenu();
	if((err = cut_out_rect(&rect)) < 0)
		goto error;
	scale_zwinmenu();
	copy_rectfields(&rect,&vl.zrect);
	save_zrectpos(); /* save new position setting */
	zwinmenu.width = rect.width*vs.zoomscale + ZBOX_BORSIZE*2;
	zwinmenu.height = rect.height*vs.zoomscale 
							+ ZBOX_BORSIZE + zoom_dragbar.height;

	recenter_zwinrect(); /* try to keep old center */
	bclip_rect((Rectangle *)&(zwinmenu.RECTSTART), /* but see it all */
			   (Rectangle *)&(vb.screen->wndo.RECTSTART));
error:
	softerr(err,esize_zwin_line);
	calc_open_zwinmenu(1);
	show_mp();
	return;
}

static void reposit_zoomwndo(void)
{
Errcode err;
Rectangle maxrect;
Rectangle owin;

	copy_rectfields(&vb.screen->wndo,&maxrect);
	clip_zwinrect(&maxrect);
	copy_rectfields(&zwinmenu,&owin);
	close_zwinmenu();

	if((err = screen_cut_rect(vb.screen,
					(Rectangle *)&(zwinmenu.RECTSTART),&maxrect)) < 0)
	{
		goto error;
	}

	if((err = calc_open_zwinmenu(0)) < 0)
		goto error;
	return;
error:
	softerr(err,esize_zwin_line);
	copy_rectfields(&owin,&zwinmenu);
	calc_open_zwinmenu(0);
	return;
}
static void hmp_zwin_repos(void)
{
	hide_mp();
	reposit_zoomwndo();
	show_mp();
}

static Boolean is_fullsize(void)
{
	return(vl.zoomwndo != NULL 
			&& (zoom_box.width == vb.screen->wndo.width 
				  || vl.zrect.width == vb.pencel->width)
			&& (zoom_box.height == vb.screen->wndo.height 
				  || vl.zrect.height == vb.pencel->height));
}
static void see_fullsize(Button *b)
{
	mb_set_hilite(b,is_fullsize());
	ccorner_text(b);
}
static void zwin_fullsize(Button *b)

/* get biggest zoom window you can get for current scale */
{
Errcode err;
Rectangle owin;
(void)b;

	if(is_fullsize())
		return;
	close_zwinmenu();
	scale_zwinmenu();
	copy_rectfields(&zwinmenu,&owin);
	copy_rectfields(&vb.screen->wndo,&zwinmenu);
	if(!clip_zwinrect((Rectangle *)&(zwinmenu.RECTSTART)))
	{
		recenter_zwinrect(); /* try to keep old window center */
		bclip_rect((Rectangle *)&(zwinmenu.RECTSTART), /* but see it all */
				   (Rectangle *)&(vb.screen->wndo.RECTSTART));
		/* this one's got borders */
		if(zwinmenu.width == vb.screen->wndo.width)
			zwinmenu.x = -ZBOX_BORSIZE;
		zwinmenu.width += ZBOX_BORSIZE*2;
		zwinmenu.height += zoom_dragbar.height + ZBOX_BORSIZE;
	}
	if((err = calc_open_zwinmenu(0)) < 0)
		goto error;
	return;
error:
	softerr(err,esize_zwin_line);
	copy_rectfields(&owin,&zwinmenu); /* try to open previous size */
	calc_open_zwinmenu(0);
	return;
}
static void feel_scale_slider(Button *b)
{
	feel_qslider(b);
	if(vl.zoomwndo == NULL)
		return;
	if( (vl.zrect.width * vs.zoomscale < zoom_box.width)
		|| (vl.zrect.height * vs.zoomscale < zoom_box.height))
	{
		close_zwinmenu();
		get_zwinrect();
		calc_open_zwinmenu(1);
	}
}
static void deltascale_zwndo(void *data, Button *b)
{
Rectangle zbox;
(void)data;
(void)b;

	if(vl.zoomwndo == NULL)
		return;

	/* gotta do a fancy clip since window may be too big to fit zoomed area */

	size_zrect(&vl.zrect,zoom_box.width,zoom_box.height);
	zbox.width = vl.zrect.width * vs.zoomscale;
	zbox.height = vl.zrect.height * vs.zoomscale;

	sclip_rect(&zbox,(Rectangle *)&(zoom_box.RECTSTART));
	zbox.x = zoom_box.x + ((zoom_box.width - zbox.width)>>1);
	zbox.y = zoom_box.y + ((zoom_box.height - zbox.height)>>1);

	draw_buttontop(&zoom_dragbar);
	draw_buttontop(&zoom_fullsize_sel);
	if (!(zbox.x == zoom_box.x
				&& zbox.y == zoom_box.y
				&& zbox.width == zoom_box.width
				&& zbox.height == zoom_box.height))
	{
		pj_set_rect(&zwinmenu.mw->w,0,zoom_box.x,zoom_box.y,zoom_box.width,
				 zoom_box.height);
	}
	pj_zoomblit(vb.pencel,vl.zrect.x,vl.zrect.y,
			 vl.zoomwndo,zbox.x,zbox.y,zbox.width,zbox.height,
			 vs.zoomscale,vs.zoomscale);
};

Boolean y_needs_zoom(Coor y)
{
if (!vs.zoom_open)
	return(FALSE);
if ((y -= vl.zrect.y) < 0)
	return(FALSE);
if (y * vs.zoomscale >= zoom_box.height)
	return(FALSE);
return(TRUE);
}

void upd_zoom_dot(Pixel c, Coor x, Coor y)
{
Coor width;
Coor height;

	if ((x -= vl.zrect.x) < 0)
		return;
	if ((y -= vl.zrect.y) < 0)
		return;
	x *= vs.zoomscale;
	y *= vs.zoomscale;
	if((width = (x + vs.zoomscale)) > zoom_box.width)
		width = zoom_box.width;
	if((width -= x) <= 0)
		return;
	if((height = (y + vs.zoomscale)) > zoom_box.height)
		height = zoom_box.height;
	if((height -= y) <= 0)
		return;
	SET_RECT(vl.zoomwndo,c,x + zoom_box.x,y + zoom_box.y,width,height);
}
/* some functions that can substitute for raster library functions for 
 * zooming note that "r" is ignored */

void zoom_put_dot(Raster *r, Pixel c, Coor x, Coor y)
/* substitute for put_dot() */
{
	(void)r;
	upd_zoom_dot(c,x,y);
}

void both_put_dot(Raster *r, Pixel c, Coor x, Coor y)
/* substitute for put_dot() */
{
	upd_zoom_dot(c,x,y);
	CPUT_DOT(r,c,x,y);
}
static Tcolxldat *_zhseg_tcxl;
void zoom_put_hseg(Raster *r, Pixel *pixbuf, Coor x, Coor y, Ucoor width)
/* puts an hseg into zoom window in pencel coords */
{
UBYTE *pixel;
Coor maxx;
Coor height;
Pixel *xlat;
Pixel tcolor;
Pixel pix;
(void)r;

	if (((y -= vl.zrect.y) < 0)
		|| (y >= vl.zrect.height))
	{
		return;
	}
	if ((x -= vl.zrect.x) >= vl.zrect.width)
		return;

	if(x < 0)
	{
		width += x;
		pixel = OPTR(pixbuf,-x);
		x = 0;
	}
	else
		pixel = pixbuf;

	if(((Coor)width) <= 0)
		return;

	maxx = x + width;

	x = x * vs.zoomscale; /* get scaled values */
	y = y * vs.zoomscale;
	maxx = maxx * vs.zoomscale;

	if(maxx > zoom_box.width)
		maxx = zoom_box.width;
	maxx -= vs.zoomscale;

	/* line may be thinner if at bottom */

	if((height = y + vs.zoomscale) > zoom_box.height)
		height = zoom_box.height;
	height -= y;

	maxx += zoom_box.x;
	x += zoom_box.x;
	y += zoom_box.y;
	width = vs.zoomscale;

	if(_zhseg_tcxl) /* test for in doing a zoom_txlblit */
	{
		xlat = _zhseg_tcxl->xlat;
		tcolor = _zhseg_tcxl->tcolor;

		for(;;)
		{
			if((pix = *pixel++) != tcolor)
			{
				if(x < maxx)
					SET_RECT(vl.zoomwndo,pix[xlat],x,y,width,height);
				else
				{
					if((Coor)(width -= (x - maxx)) > 0)
						SET_RECT(vl.zoomwndo,pix[xlat],x,y,width,height);
					break;
				}
			}
			x += vs.zoomscale;
		}
	}
	else
	{
		for(;;)
		{
			if(x < maxx)
				SET_RECT(vl.zoomwndo,*pixel++,x,y,width,height);
			else
			{
				if((Coor)(width -= (x - maxx)) > 0)
					SET_RECT(vl.zoomwndo,*pixel,x,y,width,height);
				break;
			}
			x += vs.zoomscale;
		}
	}
}
void zoom_txlatblit(Raster *src, Coor sx, Coor sy, 
					Ucoor width, Ucoor height,
	                Coor dx, Coor dy, Tcolxldat *tcxl ) 

/* this not used outside of cursor.c via a fudge
 * it will take the tcxl from _zhseg_tcxl */
{
Pixel *lbuf;
Pixel stackbuf[1024/sizeof(Pixel)];

	_zhseg_tcxl = tcxl;

	if(src->type == RT_BYTEMAP)
	{
	#define Src ((Bytemap *)src)

		lbuf = Src->bm.bp[0] + sy*Src->bm.bpr + sx;
		while (height--)
		{
			zoom_put_hseg(NULL, lbuf, dx, dy++, width);
			lbuf += Src->bm.bpr;
		}

	#undef Src
	}
	else
	{
		lbuf = stackbuf;
		if(width > Array_els(stackbuf))
		{
			if ((lbuf = pj_malloc(sizeof(Pixel)*width)) == NULL)
				return;
		}
		while(height--)
		{
			GET_HSEG(src,lbuf,sx,sy++,width);
			zoom_put_hseg(NULL, lbuf, dx, dy++, width);
		}
		if(lbuf != stackbuf)
			pj_free(lbuf);
	}
	_zhseg_tcxl = NULL;
}
void zoom_put_vseg(Raster *r, Pixel *pixbuf, Coor x, Coor y, Ucoor height)
/* puts vseg into zoom window */
{
Coor maxy, width;
Pixel *pixel;
(void)r;

	if (((x -= vl.zrect.x) < 0)
		|| (x >= vl.zrect.width))
	{
		return;
	}
	if ((y -= vl.zrect.y) >= vl.zrect.height)
		return;

	if(y < 0)
	{
		height += y;
		pixel = OPTR(pixbuf,-y);
		y = 0;
	}
	else
		pixel = pixbuf;

	if(((Coor)height) <= 0)
		return;

	maxy = y + height;
	x *= vs.zoomscale;
	y *= vs.zoomscale;
	maxy *= vs.zoomscale;

	if(maxy > zoom_box.height)
		maxy = zoom_box.height;

	/* line may be thinner if at right */

	if((width = x + vs.zoomscale) > zoom_box.width)
		width = zoom_box.width;
	width -= x;

	maxy += (zoom_box.y - vs.zoomscale);
	x += zoom_box.x;
	y += zoom_box.y;
	height = vs.zoomscale;

	for(;;)
	{
		if(y < maxy)
			SET_RECT(vl.zoomwndo,*pixel++,x,y,width,height);
		else
		{
			if((Coor)(height -= (y - maxy)) > 0)
				SET_RECT(vl.zoomwndo,*pixel,x,y,width,height);
			break;
		}
		y += vs.zoomscale;
	}
}

void
zoom_blitrect(Raster *src, Coor sx, Coor sy,
		Coor x, Coor y, Coor width, Coor height)
/* does not clip to source rectangle only to zoom window since zoom window
 * is within vb.pencel it is safe for that others must be valid source
 * rects */
{
Coor zx, zy, flix, fliy;

	if(!(vs.zoom_open && vl.zoomwndo))
		return;

	if((flix = x - vl.zrect.x) >= vl.zrect.width)
		return;
	if((fliy = y - vl.zrect.y) >= vl.zrect.height)
		return;
	if(flix < 0)
	{
		width += flix;
		flix = 0;
	}
	if(fliy < 0)
	{
		height += fliy;
		fliy = 0;
	}
	zx = flix * vs.zoomscale;
	flix += vl.zrect.x;
	zy = fliy * vs.zoomscale;
	fliy += vl.zrect.y;

	sx += flix - x;
	sy += fliy - y;

	width *= vs.zoomscale;
	height *= vs.zoomscale;
	if((width += zx) > zoom_box.width)
		width = zoom_box.width;
	if((width -= zx) <= 0)
		return;

	if((height += zy) > zoom_box.height)
		height = zoom_box.height;
	if((height -= zy) <= 0)
		return;

	pj_zoomblit(src,sx,sy,
			 vl.zoomwndo,
			 zx + zoom_box.x, zy + zoom_box.y, width, height,
			 vs.zoomscale,vs.zoomscale);
}
void rect_zoom_it(Coor x,Coor y, Coor width, Coor height)

/* like zoom_it() but does small rects */
{
	zoom_blitrect(((Raster *)vb.pencel),x,y,x,y,width,height);
}
/**** tcolor translate zoom raster *****/

static Errcode hand_ptfunc(Pentool *pt, Wndo *w)
{
	(void)pt;

	if(w == vl.zoomwndo)
		return scroll_zoomwndo();
	else
		return move_penwndo();
}

static Pentool hand_ptool = PTOOLINIT1(
	NONEXT,
	"",
	PTOOL_OPT,
	0,
	NULL,
	NO_SUBOPTS,
	NULL,
	hand_ptfunc,
	&hand_cursor.hdr,
	NULL, /* on install */
	NULL /* on remove */
);

static Smu_button_list zoom_smblist[] = {
	{ "title",  { &zoom_tit_sel } },
	{ "src",    { &zoom_source_sel } },
	{ "dest",   { &zoom_resize_sel } },
	{ "fullsz", { &zoom_fullsize_sel } },
	{ "scale",  { &zoom_scaletext } },
};

void go_zoom_settings(void)
{
Pentool *optool;
void *ss;

	if(zoom_disabled())
		return;
	hide_mp();
	fliborder_on();
	if(soft_buttons("zoom_panel", zoom_smblist, 
				    Array_els(zoom_smblist),&ss) < Success)
	{
		goto error;
	}
	optool = vl.ptool;
	set_curptool(&hand_ptool);
	menu_to_cursor(vb.screen,&zoom_menu);
	do_menuloop(vb.screen,&zoom_menu,NULL,NULL,check_pen_abort);
	restore_pentool(optool);
	smu_free_scatters(&ss);
error:
	show_mp();
}

void zoom_it(void)
{
	if(vs.zoom_open && vl.zoomwndo)
	{
		pj_zoomblit(vb.pencel,vl.zrect.x,vl.zrect.y,
				 vl.zoomwndo,zoom_box.x,zoom_box.y,zoom_box.
				 width,zoom_box.height,
				 vs.zoomscale,vs.zoomscale);
	}
}
static void set_zoom(void)
{
	hide_mp();
	get_zwinrect();
	calc_zoombox();
	if((rect_in_place(&vl.zrect)) >= 0
		&& (clip_move_rect(&vl.zrect)) >= 0)
	{
		save_zrectpos(); /* save new position setting */
	}
	open_zwinmenu(1);
	show_mp();
}

static SHORT lzoom_mode, zstack;
Boolean zoom_disabled(void)
{
	return(zstack > 0);
}
void toggle_zoom(Button *m)
{
	if(zstack > 0)
		return;
	if(vl.zoomwndo)
		close_zwinmenu();
	else
		set_zoom();
	draw_buttontop(m);
}
void ktoggle_zoom(void)
{
	if(zstack > 0)
		return;
	if(vl.zoomwndo)
		close_zwinmenu();
	else
		set_zoom();
}
Boolean zoom_hidden(void)
{
	return(zstack < 0 && lzoom_mode);
}
void unzoom(void)
/* temporarily get out of zoom (used by system not user) */
{
	if(zstack <= 0)
	{
		if(vs.zoom_open)
		{
			lzoom_mode = vs.zoom_open;
			close_zwinmenu();
		}
		zstack = 0;
	}
	++zstack;
}
void rezoom(void)
/* go back to zoom from temporary suspension note this is also used for 
 * "rezooming" on startup */
{
	if (--zstack <= 0)
	{
		if(lzoom_mode)
		{
			get_zwinrect();
			calc_open_zwinmenu(1);
			lzoom_mode = 0;
		}
	}
}

void init_zoom(void)
/* used on startup to initialize zoom mode if flag was saved */
{
	/* initialize local rectangle and open zoom window if was open */
	lzoom_mode = vs.zoom_open;
	zstack = 1;
	rezoom();
}

