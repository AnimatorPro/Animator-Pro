#define RASTCALL_INTERNALS
#define WNDO_INTERNALS
#include <limits.h>  /* for SHRT_MAX etc */
#include <stdio.h>
#include "errcodes.h"
#include "gfx.h"
#include "input.h"
#include "marqi.h"
#include "memory.h"
#include "ptrmacro.h"
#include "reqlib.h"
#include "wndo.h"

typedef void (*Bhblit)(Raster *behind,Coor x,Coor y,
		Raster *rast, Coor rx,  Coor ry, Ucoor width, Ucoor height);

static void blit_saveonly(Raster *behind,Coor x,Coor y,
						  Raster *rast,Coor rx, Coor ry,
						  Ucoor width, Ucoor height)
{
	/* simply move raster to behind no need to copy in rast to its areas */
	pj_blitrect(rast,rx,ry,behind,x,y,width,height);
}


static void blit_behind(Wndo *w,Bhblit blit)

/* blits window behind to restore others or restore others and save itself
 * when moving or closing a window */
{
SHORT x,y;
SHORT nextx, nexty;
SHORT xmax, ymax;
LONG firstx;
LONG osy;
UBYTE *ydots;
int rastid;
Raster *rast;

	xmax = w->W_xmax - w->behind.x;
	ymax = w->W_ymax - w->behind.y;
	y = w->y - w->behind.y;
	firstx = w->x - w->behind.x;

	for(;;)
	{
		x = firstx;
		ydots = w->ydots[x];
		rastid = ydots[y];
		osy = y + w->behind.y;
		nexty = w->vchanges[y];
		for(;;)
		{
			nextx = NEXTX(ydots);
			if(nextx >= xmax) /* equal will always be */
			{
				if(rastid != w->W_rastid)
				{
					rast = w->rasts[rastid];
					(*blit)(&w->behind,x,y,rast,
							(x + w->behind.x - rast->x),
							(osy - rast->y),
							(xmax - x), (nexty - y) );
				}
				if(nexty >= ymax)
					return;
				break; /* break for nextx loop */
			}
			ydots = w->ydots[nextx]; /* get next ydots for nextx and rastid */
			if(rastid == ydots[y]) /* if same continue and check next one */
				continue;
			if(rastid != w->W_rastid)
			{
				rast = w->rasts[rastid];
				(*blit)(&w->behind,x,y,rast,
						  (x + w->behind.x - rast->x),
						  (osy - rast->y),
						  (nextx - x), (nexty - y) );
			}
			rastid = ydots[y];
			x = nextx;
		}
		y = nexty;
	}
}
static int get_newid(Wscreen *ws)

/* this doesn't check for over-run couse if all else is well it won't */
{
Raster **rasts;
int id;
	rasts = ws->wndo.rasts;
	for(id = FIRST_OPENRASTID;;++id)
		if(rasts[id] == NULL) /* allways left NULL by close window */
			return(id);
}
static void get_wndo_oset(Wndo *w, Short_xy *oset)
{
	oset->x = w->x - w->behind.x;
	oset->y = w->y - w->behind.y;
}
void redraw_wndo(Wndo *w)
{
	if(w->redraw)
		(*(w->redraw))(w);
	w->mc_csetid = w->W_screen->mc_csetid;
}
Boolean reposit_wndo(Wndo *w,Rectangle *newpos,Short_xy *newoset)

/* repositions and/or re-sizes and/or scrools, rebuilds clips and
 * redraws a window, must have either a newpos a newoset or both */
{
Fullrect pos;
Short_xy oset;
Short_xy woset;
int wasin = 0;
Wscreen *ws;

	if(w->type != RT_WINDOW) /* can't move non windows */
		return(0);

	if(newpos != NULL)
	{
		copy_rectfields(newpos,&pos);
	}
	else
	{
		pos.x = w->x;
		pos.y = w->y;
		pos.width = w->W_xmax - pos.x;
		pos.height = w->W_ymax - pos.y;
	}

	get_wndo_oset(w,&woset);

	if(newoset == NULL || w->flags & WNDO_BACKDROP)
		oset = woset;
	else
		oset = *newoset;

	ws = w->W_screen;

	/* clip position width and height */

	if(pos.width > w->behind.width)
		pos.width = w->behind.width;
	if(pos.width < WNDO_MINWIDTH)
		pos.width = WNDO_MINWIDTH;

	if(pos.height > w->behind.height)
		pos.height = w->behind.height;
	if(pos.height < WNDO_MINHEIGHT)
		pos.height = WNDO_MINHEIGHT;

	/* clip offset so offset doesn't put port outside of "behind"
	 * this give precedence to the position and will "scroll" window to fit
	 * if new size is larger than the previous offset will allow */

	/* note: negative offsets not allowed will not crash but will scroll
	 * to right edge of behind */

	if((USHORT)oset.x > (w->width - pos.width))
		oset.x = w->width - pos.width;
	if((USHORT)oset.y > (w->height - pos.height))
		oset.y = w->height - pos.height;

	rect_tofrect((Rectangle *)&pos,&pos); /* fill in MaxX and MaxY */

	if( (pos.x == w->x)
		&& (pos.y == w->y)
		&& (pos.MaxX == w->W_xmax)
		&& (pos.MaxY == w->W_ymax)
		&& (oset.x == woset.x)
		&& (oset.y == woset.y))
	{
		return(0); /* no change */
	}

	if(!(w->flags & WNDO_HIDDEN)
		&& (wasin = crects_overlap((Cliprect *)&(w->CRECTSTART),
								   (Cliprect *)&(ws->wndo.CRECTSTART))) != 0)
	{
		if(w->flags & WNDO_BACKDROP) /* move it's image to new place */
		{
			blitmove_rect(&(ws->wndo),w->x,w->y,
						  &(ws->wndo),pos.x, pos.y, w->width, w->height);
			set_leftbehind(&(ws->wndo),0,w->x,w->y,
						   pos.x, pos.y, w->width, w->height);
		}
		else
			blit_behind(w,pj__swaprect); /* restore screen get window contents */
	}

	w->x = pos.x;
	w->y = pos.y;
	w->behind.x = pos.x - oset.x;
	w->behind.y = pos.y - oset.y;
	w->W_xmax = pos.MaxX;
	w->W_ymax = pos.MaxY;

	if(w->flags & WNDO_HIDDEN)
		return(1);

	if(crects_overlap((Cliprect *)&(w->CRECTSTART),
					   (Cliprect *)&(ws->wndo.CRECTSTART)))
	{
		build_all_clips(ws,1);

		if(!(w->flags & WNDO_BACKDROP))
			blit_behind(w,pj__swaprect); /* restore window in new position */

		if(w->mc_csetid != ws->mc_csetid) /* redraw if new colors */
			redraw_wndo(w);
	}
	else if(wasin)
		build_all_clips(ws,1);

	return(1);
}

Errcode move_rear_wndo(Wndo *w, void (*clipit)(Rectangle *r))

/* this is a special case of moving a "rear" window onto the screen where
 * the windows backup raster is not considered to have any essential info
 * in it so the window may be scrolled smoothly across the screen */
{
Wscreen *ws = w->W_screen;
SHORT firstx, firsty, lastx, lasty;
Rectangle opos;
Rectangle newpos;

	if(w->type == RT_ROOTWNDO)
		return(0);

	copy_rectfields(w,&opos); /* save original position */

	/* this will put window contents into its backup area */
	blit_behind(w,blit_saveonly);
	w->flags |= WNDO_HIDDEN; /* we got window all in its backup area */
	build_all_clips(ws,1); /* this makes the screen wndo drawable */
	firstx = icb.sx;
	firsty = icb.sy;
	newpos = opos;

	for(;;)
	{
		if (JSTHIT(MBRIGHT|KEYHIT)) /* Cancel - revert to old position */
			break;
		wait_input(MMOVE|MBPEN|MBRIGHT|KEYHIT);
		if(JSTHIT(MBPEN))	/* Accepted pendown - use new position */
			break;
		else if (JSTHIT(MBRIGHT|KEYHIT))
			{
			newpos.x = opos.x;
			newpos.y = opos.y;
			}
		else
			{
			newpos.x = opos.x + (icb.sx - firstx);
			newpos.y = opos.y + (icb.sy - firsty);
			}

		if(clipit)
			(*clipit)(&newpos);

		lastx = w->x;
		lasty = w->y;
		if(!reposit_wndo(w,&newpos,NULL))
			continue;

		/* blit port onto screen (clips) */

		pj_blitrect(&(w->behind),w->x - w->behind.x,w->y - w->behind.y,
				 (Raster *)&(ws->wndo), w->x, w->y,
				 w->W_xmax - w->x, w->W_ymax - w->y);


		/* if window not obscureing screen an edge is showing */

		if(CODEOBSCURES !=
			clipcode_crects((Cliprect *)&(w->CRECTSTART),
							 (Cliprect *)&(ws->wndo.CRECTSTART)))
		{
			/* clear any area left uncovered by last blit */
			set_leftbehind(&(ws->wndo),0,lastx,lasty,
						   w->x,w->y,w->W_xmax - w->x,w->W_ymax - w->y);
		}
	}

	w->flags &= ~WNDO_HIDDEN; /* we got window all in its backup area */
	build_all_clips(ws,1); /* this makes the screen wndo drawable */
	return(0);
}

static void wndohide(Wndo *w,USHORT full_update)
/* makes window invisible but maintains position in list */
{
Wscreen *ws;

	if((NULL == w->W_node.next)
		|| w->flags & WNDO_HIDDEN)
	{
		return;
	}
	ws = w->W_screen;
	blit_behind(w,pj__swaprect); /* save window restore screen */
	w->flags |= WNDO_HIDDEN;
	build_all_clips(ws,full_update);
}
static void wndoshow(Wndo *w,USHORT full_update)
/* restores a hidden window to visibility */
{
Wscreen *ws;

	if((NULL == w->W_node.next)
		|| !(w->flags & WNDO_HIDDEN))
	{
		return;
	}
	ws = w->W_screen;
	w->flags &= ~(WNDO_HIDDEN);
	build_all_clips(ws,full_update);
	blit_behind(w,pj__swaprect); /* save screen and restore window */
	if(full_update && w->mc_csetid != ws->mc_csetid) /* redraw if new colors */
		redraw_wndo(w);
}
void set_wndo_cursor(Wndo *w, Cursorhdr *ch)
{
	w->cursor = ch;
	set_cursor(ch);
}
void save_wiostate(Wiostate *ws)

/* used to save icb state info of current window cursor and settings
 * should only be used when paired with rest_wiostate() and should not bracket
 * operations that could close or hide a window since the cursor or
 * other window dependent data may be invalidated */
{
	get_mouset(&(ws->mset));
	ws->procmouse = icb.procmouse;
	ws->cursor = icb.curs;
	ws->iowndo = icb.iowndo;
}
void rest_wiostate(Wiostate *ws)

/* restores icb state info saved with save_wiostate() */
{
	icb.procmouse = ws->procmouse;
	set_cursor(ws->cursor);
	icb.iowndo = ws->iowndo;
	load_mouset(&(ws->mset));
}
void load_wndo_mouset(Wndo *w)

/* sets input mouse and macro settings to those for the window */
{
	if(w == NULL) /* set to screen values note: input screen must be set! */
	{
		w = &(icb.input_screen->wndo);
		icb.procmouse = NULL;
		icb.iowndo = NULL;
		icb.mset.wndoid = 0;
	}
	else
	{
		icb.procmouse = w->procmouse;
		icb.iowndo = w;
		icb.mset.wndoid = w->userid;
	}

	icb.mset.oset.x = w->behind.x;
	icb.mset.oset.y = w->behind.y;
	reset_icb();
}
void load_wndo_iostate(Wndo *w)

/* sets all window specific io control to the window to screen defaults if
 * NULL */
{
	load_wndo_mouset(w);
	if(w != NULL)
		set_cursor(w->cursor);
	else
		set_cursor(icb.input_screen->cursor);
}
Boolean marqmove_wndo(Wndo *w, Rectangle *bclip)

/* does a marqi rectangle on screen and moves window to result
 * returns 0 if window was not moved 1 if moved window must be open
 * and unhidden !! */
{
Rectangle newpos;
Marqihdr md;
Wscreen *ws= w->W_screen;
Boolean ret;
Wiostate ios;

	save_wiostate(&ios);
	disable_wtasks();

	load_wndo_mouset(NULL); /* to screen */
	if(!(w->flags & WNDO_BACKDROP))
		wndohide(w,0);

	copy_rectfields(w,&newpos);
	init_marqihdr(&md, (Wndo *)ws->viscel, NULL, ws->SWHITE, ws->SBLACK);
	if(marqmove_rect(&md, &newpos, bclip) < 0)
		ret = 0;
	else
	{
		ret = reposit_wndo(w,&newpos,NULL);
		if(ios.iowndo == w) /* if iostate was for this window update offset */
		{
			ios.mset.oset.x = w->behind.x;
			ios.mset.oset.y = w->behind.y;
		}
	}

	if(!(w->flags & WNDO_BACKDROP))
		wndoshow(w,1);

	enable_wtasks();
	rest_wiostate(&ios);
	return(ret);
}
void _close_wndo(Wndo *w)
{
Wscreen *ws;
int screen_wndo;
Boolean was_mouse;

	if(w == NULL)
		return;

	was_mouse = hide_mouse();
	ws = w->W_screen;
	screen_wndo = (&ws->wndo == w);
	/* restore screen behind window if not the screen window and its attached
	 * to the screen */

	if(!screen_wndo && w->W_node.next)
	{
		if(!(w->flags & WNDO_HIDDEN))
		{
			if(w->flags & WNDO_BACKDROP)
				pj_set_rast((Raster *)w, 0); /* clear the area left behind */
			else
				blit_behind(w,pj_blitrect); /* blit back and
										  * restore window behind */
		}

		if(w == icb.iowndo)
		{
			set_cursor(ws->cursor);
			set_procmouse(NULL);
			icb.iowndo = NULL;
		}
		rem_node(&(w->W_node));

		if(!(w->flags & WNDO_BACKDROP)) /* backdrops don't affect clips */
		{
			ws->wndo.rasts[w->W_rastid] = NULL; /* clear pointer for id */
			build_all_clips(ws,1);
			--ws->num_wins;
		}
	}
	pj_close_raster(&w->behind);
	pj_gentle_free(w->ydots);
	pj_gentle_free(w->vchanges);
	if(!screen_wndo)
		pj_free(w);
	if(was_mouse)
		show_mouse();
}
void close_wndo(Wndo *w)
{
	if(w == NULL || w->type == RT_ROOTWNDO)
		return;
	_close_wndo(w);
}

static Cliphead *link_free_clips(Cliphead *first,LONG clipsize,LONG allocsize)
{
register Cliphead *clip;
register Cliphead *max_clip;

	clip = first;
	max_clip = OPTR(clip,allocsize);

	for(;;)
	{
		if((clip->nextfree = OPTR(clip,clipsize)) >= max_clip)
			break;
		clip = clip->nextfree;
	}
	clip->nextfree = NULL;
	return(first);
}

void *get_free_clip(Cliphead **freeclips)
{
Cliphead *free;

	if((free = *freeclips) == NULL)
	{
		/* boxf("out of free ydots !!!"); */
		fprintf(stderr, "out of free ydots !!!");
		return(NULL);
	}
	*freeclips = free->nextfree;
	return((void *)(free + 1));

#ifdef FORTESTING
	free = pj_malloc((TOSTRUCT(Wndo,free_ydots,freeclips)->behind.height)
				+ (1 * sizeof(Cliphead)) );
/*	boxf("get clip %lx", free + 1); */
	return(free + 1);
#endif
}
void add_free_clip(Cliphead **freeclips, void *clip)
{
#define CLIP ((Cliphead *)clip)

	clip = CLIP - 1;
	CLIP->nextfree = *freeclips;
	*freeclips = CLIP;

#ifdef FORTESTING
/*	boxf("freeing %lx", clip); */
	pj_free(CLIP - 1);
#endif

#undef CLIP
}

Errcode open_wndo(Wndo **pw, WndoInit *wi)
/*
 * Open a window.  Returns window in *pw.  
 * (pw == NULL is valid input, but should only be used by
 * open_wscreen()).
 */
{
Errcode err;
Wndo *w = NULL;
Wscreen *ws;
Boolean was_mouse;

LONG allocsize;
LONG ydotsize;
LONG ptrsize;
SHORT screen_wndo;
SHORT is_backdrop;
SHORT max_wins;

	was_mouse = hide_mouse();
	is_backdrop = wi->flags & WNDO_BACKDROP;
	screen_wndo = (pw == NULL);
	ws = wi->screen;

	max_wins = ws->max_wins;

	if(screen_wndo)
	{
		w = &ws->wndo;
		w->type = RT_ROOTWNDO;
		w->cmap = ws->viscel->cmap;
		w->W_rastid = NULL_RASTID;
	}
	else
	{
		*pw = NULL;
		if(!is_backdrop)
		{
			if(ws->num_wins >= max_wins)
			{
				err = Err_tomany_wins;
				goto error;
			}
			--max_wins;  /* these windows can't be behind backdrops */
		}
		if((w = pj_zalloc(sizeof(Wndo) + wi->extrabuf)) == NULL)
			goto nomem_error;
		w->type = RT_WINDOW;
		w->rasts = ws->wndo.rasts;
		w->cmap = ws->wndo.cmap;

		if(is_backdrop)
			w->W_rastid = NULL_RASTID; /* the initial null raster */
		else
		{
			w->W_rastid = get_newid(ws);
			w->rasts[w->W_rastid] = &(w->behind);
		}
	}

	if(wi->cursor == NULL)
		w->cursor = ws->cursor;
	else
		w->cursor = wi->cursor;

	w->W_screen = ws; /* must get to here ok if window is allocated */

	/* move in raster header fields from root screen raster */

	w->aspect_dx = ws->viscel->aspect_dx;
	w->aspect_dy = ws->viscel->aspect_dy;
	w->pdepth	 = ws->viscel->pdepth;

	copy_rectfields(wi,w); /* copy in rectangle fields for initial size */

	/* none too small */

	if(w->width < WNDO_MINWIDTH)
		w->width = WNDO_MINWIDTH;
	if(w->height < WNDO_MINHEIGHT)
		w->height = WNDO_MINHEIGHT;

#ifdef DOESNTWORK
	/* no window ports bigger than screen !! */

	/* this should be a and cliprects type of clip to get common rect
	 * if needed */

	sclip_rect((Rectangle *)&(w->RECTSTART),
			   (Rectangle *)&(ws->viscel->RECTSTART));

#endif /*  DOESNTWORK */

	/* load cliprect variables for window port position and size */

	w->W_xmax = w->x + w->width;
	w->W_ymax = w->y + w->height;

	/* increase width and height of window to be the maximum size
	 * requested if greater than port size */

	if(wi->maxw > w->width)
		w->width = wi->maxw;
	if(wi->maxh > w->height)
		w->height = wi->maxh;


	/* initialize and copy in window maxsize rectangle fields to behind rast
	 * the width and height for both shall NEVER be changed until window is
	 * closed */

	copy_rasthdr(ws->viscel,&w->behind);
	copy_rectfields(w,&w->behind);

	if(is_backdrop)
	{
		if((err = pj_open_nullrast(&w->behind)) < 0) /* dummy backup area */
			goto error;
	}
	else
	{
		if((err = pj_open_bytemap((Rasthdr *)&w->behind,(Bytemap *)&w->behind)) < 0)
			goto error;
	}

	/* size of one column array of ydots */

	ydotsize = (w->height	  /* one byte for each dot */
				+ (w->height & 1) /* even word size */
				+ sizeof(Cliphead));	 /* cliphead for range and freelist */

	/* size of all ydots buffers */

	allocsize = (ydotsize * ((max_wins * 2) - 1));
	/* size of pointer array for ydots */

	/* size of pointer array */

	ptrsize = w->width * sizeof(PTR);

	if((w->ydots = pj_malloc(allocsize + ptrsize)) == NULL)
		goto nomem_error;

	w->free_ydots = link_free_clips(OPTR(w->ydots,ptrsize),ydotsize,allocsize);

	w->ydots[0] = get_free_clip(&w->free_ydots);
	NEXTX(w->ydots[0]) = w->width;
	free_init_ydots(w);

	/* allocate vchanes */
	if((w->vchanges = pj_malloc(w->height * sizeof(SHORT))) == NULL)
		goto nomem_error;

	pj_stuff_words(w->height,w->vchanges,w->height);

	w->lib = get_window_lib();

	/******** add window to screen ********/

	if(screen_wndo)
	{
		add_head(&ws->wilist,&w->W_node);
		build_all_clips(ws,0);
		++ws->num_wins;
		return(0);
	}

	w->flags |= wi->flags & ~(WNDO_NOCLEAR);

	if(is_backdrop)
	{
#ifdef WASTHISWAY
		insert_before(&(w->W_node),&(ws->wndo.W_node));
#endif
		if( (wi->over != NULL)
			 && (find_header(&(wi->over->W_node)) == &(ws->wilist)))
		{
			insert_before(&(w->W_node),&(wi->over->W_node));
		}
		else
		{
			add_head(&ws->wilist,&w->W_node);
		}
		build_all_clips(ws,1);
		if(!(wi->flags & WNDO_NOCLEAR))
			pj_set_rast((Raster *)w, 0); /* clear window */
	}
	else
	{
		++ws->num_wins;

		if( (wi->over != NULL)
			 && (find_header(&(wi->over->W_node)) == &(ws->wilist)))
		{
			insert_before(&(w->W_node),&(wi->over->W_node));
		}
		else
		{
			add_head(&ws->wilist,&w->W_node);
		}
		build_all_clips(ws,1);

		if(wi->flags & WNDO_NOCLEAR)
		{
			/* only blit in parts that are backups */
			blit_behind(w,blit_saveonly);
		}
		else
		{
			pj_set_rast(&w->behind,0);	 /* clear backup raster */
			blit_behind(w,pj__swaprect); /* swap window raster onto screen */
		}
	}

	*pw = w;
	err = Success;
	goto done;

nomem_error:
	err = Err_no_memory;
error:
	_close_wndo(w);
done:
	if(was_mouse)
		show_mouse();
	return(err);
}
Boolean ptin_wndo(Wndo *w,SHORT x,SHORT y)

/* returns if point is in window port or not */
{
	if((x < w->x)
	   || (x >= w->W_xmax)
	   || (y < w->y)
	   || (y >= w->W_ymax))
	{
		return(0);
	}
	return(1);
}
Boolean curson_wndo(Wndo *w)
/* returns 1 if processed mouse screen position is over a visible part
 * of the window */
{
	return(wndo_dot_visible(w,icb.cx - w->behind.x,icb.cy - w->behind.y));
}
Boolean mouseon_wndo(Wndo *w)
/* returns 1 if unprocessed mouse screen position is over a visible part
 * of the window */
{
	return(wndo_dot_visible(w,icb.sx - w->behind.x,icb.sy - w->behind.y));
}
Boolean wndo_dot_visible(Wndo *w,Coor x,Coor y)

/* returns 1 if point is unobscured and drawn on screen 0 if point is
 * in a backup area behind a window of off the screen */
{
	if( ((Ucoor)x) >= w->width
		|| ((Ucoor)y) >= w->height)
	{
		return(0);
	}
	return( *(w->ydots[x] + y) == 0); /* 0 is rastid of screen */
}
