/* marqi.c - routines to make creepy dotted lines around selected areas. */
/* Also some "Dotout" family functions that take x/y parameters and act
   on render_form.  Dotout's are used by line drawers, circle drawers etc. */

#define MARQI_C
#include "errcodes.h"
#include "memory.h"
#include "input.h"
#include "wndo.h"
#include "ptrmacro.h"
#include "marqi.h"


static void marqidot(SHORT x,SHORT y,register Marqihdr *mh)
/* dotout for creepy marqi users */
{
Pixel color;

	color = ((--(mh->dmod)&7) < 4 ? mh->oncolor : mh->offcolor);
	(*(mh->putdot))(mh->w,color,x,y);
}
static void oncolordot(SHORT x,SHORT y,Marqihdr *mh)
/* set dot to oncolor */
{
	(*(mh->putdot))(mh->w,mh->oncolor,x,y);
}

void init_marqihdr(Marqihdr *mh,Wndo *w,Rectangle *port,
			  Pixel oncolor,Pixel offcolor)
{
Rectangle winrect;

	clear_mem(mh, sizeof(*mh));
	mh->w = w;

	winrect.x = winrect.y = 0; /* window relative to itself */
	if(w->type == RT_WINDOW)
	{
		winrect.width = w->behind.width;
		winrect.height = w->behind.height;
	}
	else /* treat as raster */
	{
		winrect.width = w->width;
		winrect.height = w->height;
	}

	if(port != NULL) /* clip port to window and move in to mh->port */
	{
		copy_rectfields(port,&(mh->port));
		bclip_rect((Rectangle *)&(mh->port.RECTSTART),&winrect);
		rect_tofrect((Rectangle *)&(mh->port.RECTSTART),&(mh->port));
	}
	else /* port is window or raster's visible port */
	{
		rect_tofrect(&winrect,&(mh->port));
	}

	if(oncolor != offcolor) /* weze marqi'in */
	{
		mh->pdot = marqidot;
		mh->offcolor = offcolor;
		mh->waitcount = 4;
	}
	else
	{
		mh->pdot = oncolordot; /* solid lines */
		mh->waitcount = -1;
	}

	mh->putdot = pj_put_dot; /* default seting */
	mh->oncolor = oncolor;
}

/********* for marqi-ing raster rectangles when moving windows etc ********/

#define RD ((Rectdata *)(mh->adata))

typedef struct rectdata {
	Fullrect fr;
	UBYTE *save;
	Rectangle *clip; /* the boundary or size clip */
	SHORT lastx, lasty;
	SHORT saved;
} Rectdata;

static saverest_frame(Marqihdr *mh, Boolean save)

/* saves a frame (clips for safety) and has inverse restore */
{
UBYTE *sbuf;
Rectdata *rd;
VFUNC hmove, vmove;

	rd = mh->adata;
	if(save)
	{
		hmove = pj_get_hseg;
		vmove = pj_get_vseg;
		rd->saved = 1;
	}
	else /* restore */
	{
		if(!rd->saved)
			return;
		hmove = pj_put_hseg;
		vmove = pj_put_vseg;
	}

	sbuf = rd->save;
	(*hmove)(mh->w,sbuf,rd->fr.x,rd->fr.y,rd->fr.width);
	sbuf += rd->fr.width;
	(*hmove)(mh->w,sbuf,rd->fr.x,rd->fr.MaxY-1,rd->fr.width);
	sbuf += rd->fr.width;
	(*vmove)(mh->w,sbuf,rd->fr.x,rd->fr.y,rd->fr.height);
	sbuf += rd->fr.height;
	(*vmove)(mh->w,sbuf,rd->fr.MaxX-1,rd->fr.y,rd->fr.height);
}

static void marqi_mhframe(Marqihdr *mh)
/* Draw a hollow rectangle through marqidot routine */
{
Rectdata *rd = mh->adata;
	cline_frame(rd->fr.x,rd->fr.y,rd->fr.MaxX-1,rd->fr.MaxY-1,mh->pdot,mh);
}

static int clip_moverect(Marqihdr *mh)

/* increment mod delta move, clip, and Draw frame. */
{
Rectdata *rd = mh->adata;
SHORT dy;
Rectangle newrect;
SHORT dx;

	if(JSTHIT(MMOVE) || !(rd->saved))
	{
		copy_rectfields(&(rd->fr),&newrect);
		newrect.x += icb.mx - rd->lastx;
		newrect.y += icb.my - rd->lasty;

		if(rd->clip != NULL)
			bclip_rect(&newrect,rd->clip);

		dx = newrect.x - rd->fr.x;
		dy = newrect.y - rd->fr.y;

		if(dx || dy | !rd->saved)
		{
			saverest_frame(mh,0); /* restore frame */
			rect_tofrect(&newrect,&rd->fr);
			rd->lastx += dx;
			rd->lasty += dy;
			saverest_frame(mh,1);
		}
	}
	mh->dmod = mh->smod++;
	marqi_mhframe(mh);
	return(0);
}

Errcode marqmove_rect(Marqihdr *mh, Rectangle *rect, Rectangle *bclip)

/* marqmove_rect.  Will display marqi-ing rectangle in raster following cursor 
 * until next click if bclip is there rectangle must be entirely inside
 * the marquidata raster or it will be shrunk to the size of the raster
 * port */
{
Rectdata rd;

	rect_tofrect(rect,&rd.fr);
	if(NULL == (rd.save = pj_malloc((rect->width + rect->height)*2 )))
		return(Err_no_memory);
	mh->adata = &rd;
	rd.clip = bclip;

	rd.lastx = icb.mx;
	rd.lasty = icb.my;
	rd.saved = 0;

	anim_wait_input(KEYHIT|MBRIGHT|MBPEN,MMOVE,mh->waitcount,clip_moverect,mh);

	saverest_frame(mh,0); /* restore frame */
	pj_free(rd.save); /* free backup buffer */

	if(JSTHIT(MBPEN))
	{
		rect->x = rd.fr.x; /* we waaant it baaaybee */
		rect->y = rd.fr.y;
		return(0);
	}
	else
		return(Err_abort);
}

static int clip_rubrect(Marqihdr *mh)

/* increment mod delta move, clip, and Draw frame. */
{
Rectdata *rd = mh->adata;
Rectangle newrect;

	if(JSTHIT(MMOVE) || !(rd->saved))
	{
		frame_torect(rd->lastx, rd->lasty, icb.mx, icb.my, &newrect);
		sclip_rect(&newrect,rd->clip);

		if(newrect.width != rd->fr.width 
			|| newrect.height != rd->fr.height
			|| !rd->saved)
		{
			saverest_frame(mh,0); /* restore frame */
			rect_tofrect(&newrect,&rd->fr);
			saverest_frame(mh,1);
		}
	}
	mh->dmod = mh->smod++;
	marqi_mhframe(mh);
	return(0);
}
Errcode marqrub_rect(Marqihdr *mh, Rectangle *rect, Rectangle *sclip)

/* marqmove_rect.  Will display marqi-ing rubber rect following cursor 
 * until next click if clipit the clipit function is called after each delta
 * and before redisplay will clip to maximum width and height in sclip */
{
Rectdata rd;

	if(!sclip)
		sclip = (Rectangle *)&(mh->port.RECTSTART);

	if(NULL == (rd.save = pj_malloc((sclip->width + sclip->height)*2 )))
		return(Err_no_memory);

	mh->adata = &rd;
	rd.clip = sclip;
	rd.lastx = icb.mx;
	rd.lasty = icb.my;
	rd.saved = 0;

	anim_wait_input(KEYHIT|MBRIGHT|MBPEN,MMOVE,mh->waitcount,clip_rubrect,mh);

	saverest_frame(mh,0); /* restore frame */
	pj_free(rd.save); /* free backup buffer */

	if(JSTHIT(MBPEN))
	{
		copy_rectfields(&rd.fr,rect);
		return(0);
	}
	else
		return(Err_abort);
}

void marqi_cut(Marqihdr *mh,Coor x,Coor y)
{
	mh->dmod = mh->smod++;
	pj_cline(mh->port.x, y, x, y, mh->pdot, mh);
	mh->dmod = mh->smod;
	pj_cline(mh->port.MaxX-1, y,x, y, mh->pdot, mh);
	mh->dmod = mh->smod;
	pj_cline(x, mh->port.y, x, y, mh->pdot, mh);
	mh->dmod = mh->smod;
	pj_cline(x, mh->port.MaxY-1,x, y, mh->pdot,mh);
}
static void saverest_cut(Marqihdr *mh, int save)
{
Rectdata *rd = mh->adata;
void (*hseg)(void *r,void *buf,Coor x,Coor y,Coor w);
void (*vseg)(void *r,void *buf,Coor x,Coor y,Coor h);


	if(save)
	{
		hseg = pj_get_hseg;
		vseg = pj_get_vseg;
		rd->saved = 1;
	}
	else
	{
		hseg = pj_put_hseg;
		vseg = pj_put_vseg;
	}
	hseg(mh->w,rd->save,0,rd->lasty,mh->port.width);
	vseg(mh->w,rd->save + mh->port.width,rd->lastx,0,mh->port.height);
}

static int anim_cut(Marqihdr *mh)

/* increment mod delta move, clip, and Draw a cut cursor */
{
Rectdata *rd = mh->adata;

	if(JSTHIT(MMOVE) || !(rd->saved))
	{
		if(rd->saved)
			saverest_cut(mh,0);
		rd->lastx = icb.mx;
		rd->lasty = icb.my;
		saverest_cut(mh,1);
	}
	marqi_cut(mh,rd->lastx,rd->lasty);
	return(0);
}
static Errcode marq_getcut(Marqihdr *mh)

/* marqmove_rect.  Will display marqi-ing cut cursor and returns
 * x and y unless keyhit or right click then Err_abort returned */
{
Rectdata rd;

	if(NULL == (rd.save = pj_malloc((mh->port.width + mh->port.height)*2 )))
		return(Err_no_memory);

	mh->adata = &rd;
	rd.saved = 0;

	anim_wait_input(KEYHIT|MBRIGHT|MBPEN,MMOVE,mh->waitcount,anim_cut,mh);

	saverest_cut(mh,0);
	pj_free(rd.save); /* free backup buffer */

	if(JSTHIT(MBPEN))
		return(0);
	else
		return(Err_abort);
}
Errcode mh_cut_rect(Marqihdr *mh,Rectangle *rect,Rectangle *sclip)
{
Errcode err;

	if((err = marq_getcut(mh)) >= 0)
		err = marqrub_rect(mh,rect,sclip);
	return(err);
}
Errcode screen_cut_rect(Wscreen *s,Rectangle *rect,Rectangle *sclip)
{
Errcode err;
Marqihdr mh;
Wiostate wio;

	save_wiostate(&wio);
	set_mouse_oset(0,0);

	init_marqihdr(&mh,(Wndo *)(s->viscel),NULL,s->SWHITE,s->SBLACK);
	err = mh_cut_rect(&mh,rect,sclip);
	rest_wiostate(&wio);
	return(err);
}


