#define CURSOR_C
#include "jimk.h"
#include "brush.h"
#include "errcodes.h"
#include "fli.h"
#include "pentools.h"
#include "picfile.h"
#include "rastcurs.h"
#include "resource.h"

typedef struct curslist {
	Rastcursor *curs;
	char *name;
} Curslist;

extern void zoom_txlatblit(Raster *src, Coor sx, Coor sy,
		Ucoor width, Ucoor height,
		Coor dx, Coor dy, Tcolxldat *tcxl);

static int zoomcursor;
static Cursorsave umouse;

static void free_cursorcel(Cursorcel **pc)
{
Cursorcel *c;

	if((c = *pc) == NULL)
		return;

	*pc = NULL;
	pj_close_raster((Raster *)c);
	pj_free(c);
}
static Errcode get_filecursor(char *name, Cursorcel **pcurs,Rectangle *maxsave)
{
Jfile file;
Errcode err;
Pic_header pic;
Cursorcel *curs;

	*pcurs = NULL;
	if ((file = pj_open(name, JREADONLY)) == JNONE)
		goto jio_error;
	if(NULL == (curs = pj_zalloc(sizeof(*curs))))
	{
		err = Err_no_memory;
		goto error;
	}
	if((err = pj_read_pichead(file,&pic)) < Success)
		goto error;
	*pcurs = curs;

	copy_rasthdr(vb.cel_a,curs);
	curs->width = pic.width;
	curs->height = pic.height;

	if((err = pj_open_bytemap((Rasthdr *)curs,(Bytemap *)curs)) < Success)
		goto error;
	if((err = pj_read_picbody(file,&pic,(Raster *)curs,NULL)) < Success)
		goto error;

	if(maxsave->width < pic.width)
		maxsave->width = pic.width;
	if(maxsave->height < pic.height)
		maxsave->height = pic.height;

	curs->x = pic.x;
	curs->y = pic.y;

	pj_close(file);
	return(0);

jio_error:
	err = pj_ioerr();
error:
	free_cursorcel(pcurs);
	pj_close(file);
	return(err);
}

#ifdef SLUFFED
static void zo_line(SHORT j, 
				    register PLANEPTR spt,
					register SHORT xs,
					SHORT xd, SHORT yd, SHORT color)
{
	while (--j >= 0)
	{
		if (spt[xs>>3] & bit_masks[xs&7] )
			upd_zoom_dot(color,xd,yd);
		xs++;
		xd++;
	}
}
static void zoom_mask1blit(UBYTE *mbytes, SHORT sbpr, 
				           SHORT sx, SHORT sy, SHORT w, SHORT h,
			               SHORT dx, SHORT dy, 
			               Pixel color )
{
	mbytes += sy*sbpr;
	while (--h >= 0)
	{
		zo_line(w, mbytes, sx, dx, dy, color);
		dy++;
		mbytes += sbpr;
	}
}
#endif /* SLUFFED */

/*************************************************/

static void zshow_rastcursor(Rastcursor *rc)
{
Cursorcel *r = rc->cel;
Short_xy cpos;

	get_zoomcurs_flixy(&cpos);
	rc->save->r.x = (cpos.x -= r->x);
	rc->save->r.y = (cpos.y -= r->y);

	pj_blitrect(vb.pencel,cpos.x,cpos.y,rc->save,0,0,r->width,r->height); 
	zoom_txlatblit(r,0,0,r->width,r->height,cpos.x,cpos.y,get_cursor_xlat()); 
}
static void zhide_rastcursor(Cursorhdr *rastcursor)
{
	Rastcursor *rc = (Rastcursor *)rastcursor;

	zoom_blitrect(rc->save,0,0,
				  rc->save->r.x,rc->save->r.y,rc->cel->width,rc->cel->height);
}
static void zmove_rastcursor(Cursorhdr *rastcursor)
{
Rastcursor *rc = (Rastcursor *)rastcursor;
Cursorcel *r = rc->cel;
Cursorsave *save = rc->save;
Short_xy cpos;
Coor ox, oy;
Coor w, h;

	get_zoomcurs_flixy(&cpos);
	ox = save->r.x;
	oy = save->r.y;
	save->r.x = (cpos.x -= r->x);
	save->r.y = (cpos.y -= r->y);
	w = r->width;
	h = r->height;

	/* get save */
	pj_blitrect(vb.pencel,cpos.x,cpos.y,rc->save,0,0,w,h); 

	/* erase "leftover" area */
	do_leftbehind(ox,oy,cpos.x,cpos.y,w,h,(do_leftbehind_func)rect_zoom_it);

	/* "composit" new area (unclipped) */
	procblit(r,0,0,save,0,0,w,h,tbli_xlatline,get_cursor_xlat());

	/* apply to zoom window */
	zoom_blitrect(save, 0, 0,cpos.x,cpos.y,w,h); 

	/* re-get save area since composit corrupted it and hide may be next */
	pj_blitrect(vb.pencel,cpos.x,cpos.y,rc->save,0,0,w,h); 
}

/**************** Pen tool cursors,  These are zoomable *******************/
/* The convention for zooming cursors is, If the cursor is called from
 * the Zoom window zoomcursor == 1 else zoomcursor = 0
 * A pentool cursor MUST have a valid moveit function or else
 *
 * also if the display is conditional it should set the undisplay function
 * to maintain sync since conditions may change
 *
 * show_cursor(Corsorhdr *this)
 * { 
 *		if(zoomcursor)
 *			ZOOM MODE
 *		else
 *			UNZOOM MODE
 *		....
 */

static void show_either_rcursor(Cursorhdr *hdr)
/* this loads its hide function */
{
	Rastcursor *ch = (Rastcursor *)hdr;

	if(zoomcursor)
	{
		zshow_rastcursor(ch);
		hdr->hideit = zhide_rastcursor;
		hdr->moveit = zmove_rastcursor;
	}
	else
	{
		show_rastcursor(hdr);
		hdr->hideit = hide_rastcursor;
		hdr->moveit = move_rastcursor;
	}
}
static void save_under_brushcurs(void *screen, Rastcursor *rc,
							Rbrush *rb, Short_xy *cpos, Boolean zoom)
{
Cursorcel *r = rc->cel;
Short_xy mins;
SHORT width, height;


	if(r->x > rb->cent.x)
		mins.x = r->x;
	else
		mins.x = rb->cent.x;

	if(r->y > rb->cent.y)
		mins.y = r->y;
	else
		mins.y = rb->cent.y;

	width = r->width - r->x;

	if(width < (rb->width - rb->cent.x))
		width = rb->width - rb->cent.x;

	height = r->height - r->y;

	if(height < (rb->height - rb->cent.y))
		height = rb->height - rb->cent.y;

	width += mins.x;
	height += mins.y;
	
	if(width > r->width || height > r->height)
	{
		/* a bit cludgey but the sign bit is used as 
		   flag for brush outside of cursor restore area */

		width = r->width;
		height = (r->height | 0x8000); 
		mins.x = r->x;
		mins.y = r->y;
	}

	/* save brush position in brush */
	rb->rast->x = cpos->x - rb->cent.x;
	rb->rast->y = cpos->y - rb->cent.y;

	/* save cursor save position in cursor */
	rc->save->r.x = cpos->x - mins.x;
	rc->save->r.y = cpos->y - mins.y;
	rc->save->w = width;
	rc->save->h = height;
	if(!zoom)
	{
		pj_blitrect(screen,rc->save->r.x,rc->save->r.y,rc->save,0,0,
					width,height & 0x7FFF); 
		if(!(height & 0x8000))
			return;
	}
	save_ubrush(rb,screen,cpos->x,cpos->y); 
}
static void zhide_brushcurs(Cursorhdr *rastcursor)
{
Rastcursor *rc = (Rastcursor *)rastcursor;
Cursorsave *save = rc->save;

	rest_ubrush(vl.brush,vb.pencel); /* only brush in pencel, both in zoom */
	if(save->h & 0x8000) /* if brush outside of cursor we have to zoom it too */
	{
		rect_zoom_it(vl.brush->rast->x,vl.brush->rast->y,
					 vl.brush->width,vl.brush->height);
	}
	rect_zoom_it(save->r.x,save->r.y,save->w,save->h & 0x7FFF);
}
static void zshow_brushcurs(Rastcursor *rc)
{
Cursorcel *r = rc->cel;
Short_xy cpos, bpos;

	get_zoomcurs_flixy(&cpos);
	save_under_brushcurs(vb.pencel,rc,vl.brush,&cpos,TRUE);

	bpos = cpos;
	cpos.x -= r->x;
	cpos.y -= r->y;

	if(vs.use_brush)
		zoom_blit_brush(vl.brush,bpos.x,bpos.y);
	else
		upd_zoom_dot(vs.ccolor,bpos.x,bpos.y);

	zoom_txlatblit(r,0,0,r->width,r->height,cpos.x,cpos.y,get_cursor_xlat()); 

	if(vs.use_brush)
		blit_brush(vl.brush,vb.pencel,bpos.x,bpos.y);
	else
		pj_put_dot(vb.pencel,vs.ccolor,bpos.x,bpos.y);
}
static void hide_brushcurs(Cursorhdr *rastcursor)
{
Rastcursor *rc = (Rastcursor *)rastcursor;
Cursorsave *save = rc->save;

	if(save->h & 0x8000) /* if brush outside of cursor we have to do it too */
		rest_ubrush(vl.brush,vb.screen->viscel);
	pj_blitrect(save,0,0,vb.screen->viscel,save->r.x,save->r.y,save->w,
				save->h & 0x7FFF); 
	if(vs.zoom_open) /* only brush in zoom window */
	{
		rect_zoom_it(vl.brush->rast->x - vb.pencel->x,
					 vl.brush->rast->y - vb.pencel->y,
					 vl.brush->width,vl.brush->height);
	}
}
static void show_brushcurs(Rastcursor *rc)
{
Cursorcel *r = rc->cel;
Short_xy bpos, cpos;

	bpos.x = cpos.x = icb.cx;
	bpos.y = cpos.y = icb.cy;

	save_under_brushcurs(vb.screen->viscel,rc,vl.brush,&cpos,FALSE);

	cpos.x -= r->x;
	cpos.y -= r->y;

	if(vs.use_brush)
		blit_brush(vl.brush,vb.screen->viscel,bpos.x,bpos.y);
	else
		pj_put_dot(vb.screen->viscel,vs.ccolor,bpos.x,bpos.y);


	procblit(r,0,0,vb.screen->viscel,cpos.x,cpos.y,r->width,r->height,
			 tbli_xlatline,get_cursor_xlat());

	if(vs.zoom_open)
	{
		bpos.x -= vb.pencel->x;
		bpos.y -= vb.pencel->y;
		if(vs.use_brush)
			zoom_blit_brush(vl.brush,bpos.x,bpos.y);
		else
			upd_zoom_dot(vs.ccolor,bpos.x,bpos.y);
	}
}
static void show_brush_cursor(Cursorhdr *hdr)
/* selects display function and sets hide function for it's inverse */
{
	Rastcursor *ch = (Rastcursor *)hdr;

	if(vl.hide_brush)
	{
		if(zoomcursor)
		{
			zshow_rastcursor(ch);
			hdr->hideit = zhide_rastcursor;
			hdr->moveit = zmove_rastcursor;
		}
		else
		{
			show_rastcursor(hdr);
			hdr->hideit = hide_rastcursor;
			hdr->moveit = move_rastcursor;
		}
	}
	else
	{
		if(zoomcursor)
		{
			zshow_brushcurs(ch);
			hdr->hideit = zhide_brushcurs;
		}
		else
		{
			show_brushcurs(ch);
			hdr->hideit = hide_brushcurs;
		}
		hdr->moveit = gen_move_cursor;
	}
}
static void show_shape_cursor(Cursorhdr *ch)
{
	if(vs.fillp)
		show_either_rcursor(ch);
	else
		show_brush_cursor(ch);
}

static void zshow_pcel_curs(Cursorhdr *ch)
{
	(void)ch;

	zoomcursor = 1;
	(*(PENWNDO->cursor->showit))(PENWNDO->cursor);
	zoomcursor = 0;
}
static void zhide_pcel_curs(Cursorhdr *ch)
{
	(void)ch;

	zoomcursor = 1;
	(*(PENWNDO->cursor->hideit))(PENWNDO->cursor);
	zoomcursor = 0;
}
static void zmove_pcel_curs(Cursorhdr *ch)
{
	(void)ch;

	zoomcursor = 1;
	(*(PENWNDO->cursor->moveit))(PENWNDO->cursor);
	zoomcursor = 0;
}

Cursorhdr zoom_pencel_cursor = {
	zshow_pcel_curs,
	zhide_pcel_curs,
	zmove_pcel_curs,
};

Cursorhdr *set_pen_cursor(Cursorhdr *ch)
{
Cursorhdr *och;

	if(!PENWNDO)
		return(NULL);
	och = PENWNDO->cursor;
	if(icb.curs == &zoom_pencel_cursor) /* force redisplay if zoomed */
	{
		set_cursor(NULL); /* will force reset in input loop or procmouse */
		PENWNDO->cursor = ch;
	}
	else
		set_wndo_cursor(PENWNDO,ch);
	return(och);
}

static void show_ptool_curs(Cursorhdr *ch)
{
	(void)ch;
	(*(vl.ptool->cursor->showit))(vl.ptool->cursor);
}
static void hide_ptool_curs(Cursorhdr *ch)
{
	(void)ch;
	(*(vl.ptool->cursor->hideit))(vl.ptool->cursor);
}
static void move_ptool_curs(Cursorhdr *ch)
{
	(void)ch;
	(*(vl.ptool->cursor->moveit))(vl.ptool->cursor);
}
Cursorhdr pentool_cursor = {
	show_ptool_curs,
	hide_ptool_curs,
	move_ptool_curs,
};

Rastcursor fill_cursor = {
	{ show_either_rcursor, NULL, gen_move_cursor },
	NULL,
	&umouse,
};

/* note the next 3 have the same cel */

Rastcursor plain_ptool_cursor = {
	{ show_either_rcursor, NULL, gen_move_cursor },
	NULL,
	&umouse,
};

Rastcursor pen_cursor = {
	{ show_brush_cursor, NULL, gen_move_cursor },
	NULL,
	&umouse,
};

Rastcursor shape_cursor = {
	{ show_shape_cursor, NULL, gen_move_cursor },
	NULL,
	&umouse,
};

/****** end plain crosshair cursors ****/


Rastcursor text_cursor = {
	{ show_either_rcursor, NULL, gen_move_cursor },
	NULL,
	&umouse,
};

Rastcursor box_cursor = {
	{ show_shape_cursor, NULL, gen_move_cursor },
	NULL,
	&umouse,
};

Rastcursor move_tool_cursor = {
	{ show_either_rcursor, NULL, gen_move_cursor },
	NULL,
	&umouse,
};

Rastcursor sep_cursor = {
	{ show_either_rcursor, NULL, gen_move_cursor },
	NULL,
	&umouse,
};

Rastcursor edge_cursor = {
	{ show_either_rcursor, NULL, gen_move_cursor },
	NULL,
	&umouse,
};


Rastcursor spray_cursor = {
	{ show_brush_cursor, NULL, gen_move_cursor },
	NULL,
	&umouse,
};

Rastcursor star_cursor = {
	{ show_brush_cursor, NULL, gen_move_cursor },
	NULL,
	&umouse,
};

/*** non pentool cursors ***/

Rastcursor pick_cursor = {
	{ show_either_rcursor, NULL, gen_move_cursor },
	NULL,
	&umouse,
};

Rastcursor hand_cursor = {
	{ show_rastcursor, hide_rastcursor, move_rastcursor },
	NULL,
	&umouse,
};

Rastcursor menu_cursor = {
	{ show_rastcursor, hide_rastcursor, move_rastcursor },
	NULL,
	&umouse,
};

static Curslist cursortab[] = {
	{ &pen_cursor, "penbrush.cur" },
	{ &spray_cursor, "spray.cur" },
	{ &star_cursor, "star.cur" },
	{ &box_cursor, "box.cur" },
#define NUM_USE_BRUSH 4   /* number of items that may eventually call 
						   * save_ubrush() */

	{ &fill_cursor, "fill.cur" },
	{ &text_cursor, "texttool.cur" },
	{ &hand_cursor, "hand.cur" },
	{ &menu_cursor, "menu.cur" },
	{ &move_tool_cursor, "move.cur" },
	{ &sep_cursor, "sep.cur" },
	{ &edge_cursor, "edge.cur" },
	{ &pick_cursor, "pick.cur" },
};

static char is_init;

Errcode init_cursors(void)

/* sets up cursor save/restore area to match current display and loads any 
 * loadable cursors, defaults to crosshair 16 loadable if cursors absent */
{
Errcode err;
Curslist *clist;
char name_buf[PATH_SIZE];
Cursorcel *default_cel;

	if(is_init)
		return(Success);

	default_cel = get_default_cursor()->cel;
	copy_rasthdr(vb.cel_a,&umouse.r);

	/* size needed for brush cursor or default cursor */

	umouse.r.width = Max(BRUSH_MAX_WIDTH,DFLT_CURS_WID);
	umouse.r.height = Max(BRUSH_MAX_HEIGHT,DFLT_CURS_HT);

	/* offset for brush cursor center */

	umouse.r.x = -(umouse.r.width/2+1);
	umouse.r.y = -(umouse.r.height/2+1);

	clist = &cursortab[0];
	while(clist < &cursortab[Array_els(cursortab)])
	{
		if((err = get_filecursor(make_resource_name(clist->name, name_buf),
								&(clist->curs->cel),
								(Rectangle *)&(umouse.r.RECTSTART))) < 0)
		{
			clist->curs->cel = default_cel;
		}
		++clist;
	}

	if((err = pj_open_bytemap((Rasthdr *)&umouse.r,&umouse.r)) < 0)
		goto error;

	/* load cursors with same cels */
	plain_ptool_cursor.cel = shape_cursor.cel = pen_cursor.cel;

	set_cursor(&menu_cursor.hdr);
	show_mouse();	
	return(0);
error:
	cleanup_cursors();
	return(err);
}
void cleanup_cursors(void)
{
Curslist *clist;
Rastcursor *default_curs;

	is_init = FALSE;
	set_cursor(NULL);
	default_curs = get_default_cursor();
	if(vb.screen)
	{
		vb.screen->wndo.cursor = vb.screen->cursor = 
				vb.screen->menu_cursor = &default_curs->hdr;
		set_cursor(&default_curs->hdr);
	}
	pj_close_raster(&umouse.r);
	clist = &cursortab[0];
	while(clist < &cursortab[Array_els(cursortab)])
	{
		if(clist->curs->cel == default_curs->cel)
			clist->curs->cel = NULL;
		else
			free_cursorcel(&clist->curs->cel);
		++clist;
	}
}
Errcode	save_cursor(char *name,Rcel *rc,Short_xy *hot)
{
SHORT ox,oy;
Errcode err;

	ox = rc->x;
	oy = rc->y;
	rc->x = hot->x;
	rc->y = hot->y;
	err = save_pic(name,rc,0,FALSE);
	rc->x = ox;
	rc->y = oy;
	return(softerr(err,"!%s", "cant_save", name ));
}
