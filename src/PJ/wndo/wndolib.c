#include <assert.h>
#include "rastlib.h"
#include "memory.h"
#define WNDO_INTERNALS
#include "wndo.h"

static void win_put_dot(Raster *wndo, Pixel color, Coor x, Coor y)
{
Wndo *w = (Wndo *)wndo;
Raster *rast;

	if((Ucoor)x >= w->width) /* window raster coords are always 0,0 minimum */
		return;
	if((Ucoor)y >= w->height)
		return;
	rast = w->rasts[*(w->ydots[x] + y)];
	PUT_DOT(rast,color,x + w->behind.x - rast->x, y + w->behind.y - rast->y);
}
static void _win_put_dot(Raster *wndo, Pixel color, Coor x, Coor y)
{
Wndo *w = (Wndo *)wndo;
Raster *rast;

#ifdef FOR_CLIP_TEST

	color = NEXTX(w->ydots[x]) + (32 * 3); 
	color = (*(w->ydots[x] + y) + (32 * 3)) & 0xFF; 
	rast = w->rasts[0];
	color = ((w->vchanges[y] + *(w->ydots[x] + y)) + (32 * 3)) & 0xff;
#endif /* FOR_CLIP_TEST */

	rast = w->rasts[*(w->ydots[x] + y)];
	PUT_DOT(rast,color,x + w->behind.x - rast->x, y + w->behind.y - rast->y);
}
static Pixel win_get_dot(Raster *wndo, Coor x, Coor y)
{
Wndo *w = (Wndo *)wndo;
Raster *rast;

	/* window coords are always 0,0 minimum */

	if((ULONG)x >= w->width) 
		return(0);
	if((ULONG)y >= w->height)
		return(0);

	rast = w->rasts[*(w->ydots[x] + y)];
	return(GET_DOT(rast,x + w->behind.x - rast->x, y + w->behind.y - rast->y));
}
static Pixel _win_get_dot(Raster *wndo, Coor x, Coor y)
{
Wndo *w = (Wndo *)wndo;
Raster *rast;

	rast = w->rasts[*(w->ydots[x] + y)];
	return(GET_DOT(rast,x + w->behind.x - rast->x, y + w->behind.y - rast->y));
}
static void
_win_put_hseg(Raster *wndo, Pixel *pixbuf, Coor x, Coor y, Ucoor width)
{
Wndo *w = (Wndo *)wndo;
SHORT rastid;
UBYTE *ydots;
SHORT nextx;
SHORT xmax;
SHORT wdif;
Raster *rast;
SHORT osy;
assert(x >= 0 && y >= 0);

	xmax = width + x;
	osy = y + w->behind.y;
	ydots = w->ydots[x];
	rastid = ydots[y];

	for(;;)
	{
		nextx = NEXTX(ydots);
		if(nextx >= xmax)
		{
			rast = w->rasts[rastid];
			PUT_HSEG(rast,pixbuf,x + w->behind.x - rast->x,
								  osy - rast->y,xmax - x);
		    return;
		}
		ydots = w->ydots[nextx]; /* get next ydots for nextx */
		if(rastid == ydots[y]) /* if same raster continue and check next one */
			continue;
		rast = w->rasts[rastid];
		rastid = ydots[y];
		wdif = nextx - x;
		PUT_HSEG(rast,pixbuf,x + w->behind.x - rast->x,osy - rast->y,wdif);
		pixbuf += wdif;
		x = nextx;
	}
}
static void
_win_get_hseg(Raster *wndo, Pixel *pixbuf, Coor x, Coor y, Ucoor width)
{
Wndo *w = (Wndo *)wndo;
SHORT rastid;
UBYTE *ydots;
SHORT nextx;
SHORT xmax;
SHORT wdif;
Raster *rast;
SHORT osy;
assert(x >= 0 && y >= 0);

	xmax = width + x;
	osy = y + w->behind.y;
	ydots = w->ydots[x];
	rastid = ydots[y];

	for(;;)
	{
		nextx = NEXTX(ydots);
		if(nextx >= xmax)
		{
			rast = w->rasts[rastid];
			GET_HSEG(rast,pixbuf,x + w->behind.x - rast->x,
								  osy - rast->y,xmax - x);
		    return;
		}
		ydots = w->ydots[nextx]; /* get next ydots for nextx */
		if(rastid == ydots[y]) /* if same raster continue and check next one */
			continue;
		rast = w->rasts[rastid];
		rastid = ydots[y];
		wdif = nextx - x;
		GET_HSEG(rast,pixbuf,x + w->behind.x - rast->x,osy - rast->y,wdif);
		pixbuf += wdif;
		x = nextx;
	}
}
static void
_win_put_vseg(Raster *wndo, Pixel *pixbuf, Coor x, Coor y, Ucoor height)
{
Wndo *w = (Wndo *)wndo;
SHORT rastid;
UBYTE *ydots;
SHORT nexty;
SHORT ymax;
SHORT hdif;
Raster *rast;
SHORT osx;
assert(x >= 0 && y >= 0);

	ymax = y + height;
	osx = x + w->behind.x;
	ydots = w->ydots[x];
	rastid = ydots[y];
	nexty = w->vchanges[y];

	for(;;)
	{
		if(nexty >= ymax)
		{
			rast = w->rasts[rastid];
			PUT_VSEG(rast,pixbuf,osx - rast->x,
								  y + w->behind.y - rast->y,ymax - y);
		    return;
		}
		if(rastid != ydots[nexty])  /* if next not same raster do segment */
		{
			rast = w->rasts[rastid];
			rastid = ydots[nexty];
			hdif = nexty - y;
			PUT_VSEG(rast,pixbuf,osx - rast->x,
							y + w->behind.y - rast->y,hdif);
			pixbuf += hdif;
			y = nexty;
		}
		nexty = w->vchanges[nexty];
	}
}
static void
_win_get_vseg(Raster *wndo, Pixel *pixbuf, Coor x, Coor y, Ucoor height)
{
Wndo *w = (Wndo *)wndo;
SHORT rastid;
UBYTE *ydots;
SHORT nexty;
SHORT ymax;
SHORT hdif;
Raster *rast;
SHORT osx;
assert(x >= 0 && y >= 0);

	ymax = y + height;
	osx = x + w->behind.x;
	ydots = w->ydots[x];
	rastid = ydots[y];
	nexty = w->vchanges[y];

	for(;;)
	{
		if(nexty >= ymax)
		{
			rast = w->rasts[rastid];
			GET_VSEG(rast,pixbuf,osx - rast->x,
								  y + w->behind.y - rast->y,ymax - y);
		    return;
		}
		if(rastid != ydots[nexty])  /* if next not same raster do segment */
		{
			rast = w->rasts[rastid];
			rastid = ydots[nexty];
			hdif = nexty - y;
			GET_VSEG(rast,pixbuf,osx - rast->x,
						y + w->behind.y - rast->y,hdif);
			pixbuf += hdif;
			y = nexty;
		}
		nexty = w->vchanges[nexty];
	}
}
static void
_win_set_hline(Raster *wndo, Pixel color, Coor x, Coor y, Ucoor width)
{
Wndo *w = (Wndo *)wndo;
SHORT rastid;
UBYTE *ydots;
SHORT nextx;
SHORT xmax;
Raster *rast;
SHORT osy;
assert(x >= 0 && y >= 0);

	xmax = x + width;
	osy = y + w->behind.y;
	ydots = w->ydots[x];
	rastid = ydots[y];

	for(;;)
	{
		nextx = NEXTX(ydots);
		if(nextx >= xmax)
		{
			rast = w->rasts[rastid];
			SET_HLINE(rast,color,x + w->behind.x - rast->x,
								osy - rast->y,xmax - x);
		    return;
		}
		ydots = w->ydots[nextx]; /* get next ydots for nextx */
		if(rastid == ydots[y]) /* if same raster continue and check next one */
			continue;
		rast = w->rasts[rastid];
		rastid = ydots[y];
		SET_HLINE(rast,color,x + w->behind.x - rast->x,osy - rast->y,nextx - x);
		x = nextx;
	}
}
static void
_win_set_vline(Raster *wndo, Pixel color, Coor x, Coor y, Ucoor height)
{
Wndo *w = (Wndo *)wndo;
SHORT rastid;
UBYTE *ydots;
SHORT nexty;
SHORT ymax;
Raster *rast;
SHORT osx;
assert(x >= 0 && y >= 0);

	ymax = y + height;
	osx = x + w->behind.x;
	ydots = w->ydots[x];
	rastid = ydots[y];
	nexty = w->vchanges[y];

	for(;;)
	{
		if(nexty >= ymax)
		{
			rast = w->rasts[rastid];
			SET_VLINE(rast,color,osx - rast->x,
								y + w->behind.y - rast->y,ymax - y);
		    return;
		}
		if(rastid != ydots[nexty]) /* if same raster check next one */
		{
			rast = w->rasts[rastid];
			rastid = ydots[nexty];
			SET_VLINE(rast,color,osx - rast->x,
					y + w->behind.y - rast->y,nexty - y);
			y = nexty;
		}
		nexty = w->vchanges[nexty];
	}
}
static void
_win_set_rect(Raster *wndo, Pixel color,
		Coor x, Coor y, Ucoor width, Ucoor height)
/* set a rectangle to a color */
{
Wndo *w = (Wndo *)wndo;
SHORT xmax, ymax;
SHORT nextx, nexty;
LONG firstx;
LONG dheight;
SHORT osy;
UBYTE *ydots;
int rastid;
Raster *rast;
assert(x >= 0 && y >= 0);

	xmax = x + width;
	ymax = y + height;
	firstx = x;

	for(;;)
	{
		osy = y + w->behind.y;

		if((nexty = w->vchanges[y]) > ymax)
			dheight = ymax - y;
		else
			dheight = nexty - y;

		ydots = w->ydots[x];
		rastid = ydots[y];

		for(;;)
		{
			nextx = NEXTX(ydots);
			if(nextx >= xmax)
			{
				rast = w->rasts[rastid];
				SET_RECT(rast,color,x + w->behind.x - rast->x,
						  osy - rast->y,xmax - x,dheight);
				if(nexty >= ymax)
					return;
				break; /* break for nextx loop */
			}
			ydots = w->ydots[nextx]; /* get next ydots for nextx and rastid */
			if(rastid == ydots[y]) /* if same continue and check next one */
				continue;
			rast = w->rasts[rastid];
			rastid = ydots[y];
			SET_RECT(rast,color,x + w->behind.x - rast->x,
			  		  osy - rast->y,nextx - x,dheight);
			x = nextx;
		}
		x = firstx;
		y = nexty;
	}
}
static void
_win_xor_rect(Raster *wndo, Pixel color,
		Coor x, Coor y, Ucoor width, Ucoor height)
/* xors a rectangle with a color */
{
Wndo *w = (Wndo *)wndo;
SHORT xmax, ymax;
SHORT nextx, nexty;
LONG firstx;
LONG dheight;
SHORT osy;
UBYTE *ydots;
int rastid;
Raster *rast;
assert(x >= 0 && y >= 0);

	xmax = x + width;
	ymax = y + height;
	firstx = x;

	for(;;)
	{
		osy = y + w->behind.y;

		if((nexty = w->vchanges[y]) > ymax)
			dheight = ymax - y;
		else
			dheight = nexty - y;

		ydots = w->ydots[x];
		rastid = ydots[y];

		for(;;)
		{
			nextx = NEXTX(ydots);
			if(nextx >= xmax)
			{
				rast = w->rasts[rastid];
				XOR_RECT(rast,color,x + w->behind.x - rast->x,
						  osy - rast->y,xmax - x,dheight);
				if(nexty >= ymax)
					return;
				break; /* break for nextx loop */
			}
			ydots = w->ydots[nextx]; /* get next ydots for nextx and rastid */
			if(rastid == ydots[y]) /* if same continue and check next one */
				continue;
			rast = w->rasts[rastid];
			rastid = ydots[y];
			XOR_RECT(rast,color,x + w->behind.x - rast->x,
			  		  osy - rast->y,nextx - x,dheight);
			x = nextx;
		}
		x = firstx;
		y = nexty;
	}
}
static void
_win_mask1blit(UBYTE *mbytes, unsigned int mbpr, Coor mx, Coor my,
		Raster *wndo, Coor x, Coor y, Ucoor width, Ucoor height,
		Pixel oncolor)
{
Wndo *w = (Wndo *)wndo;
SHORT xmax, ymax;
SHORT nextx, nexty;
LONG firstx;
LONG firstmx;
LONG dheight;
SHORT osy;
UBYTE *ydots;
int rastid;
Raster *rast;

	xmax = x + width;
	ymax = y + height;
	firstx = x;
	firstmx = mx;

	for(;;)
	{
		osy = y + w->behind.y;
		if((nexty = w->vchanges[y]) > ymax)
			dheight = ymax - y;
		else
			dheight = nexty - y;

		ydots = w->ydots[x];
		rastid = ydots[y];

		for(;;)
		{
			nextx = NEXTX(ydots);
			if(nextx >= xmax)
			{
				rast = w->rasts[rastid];
				MASK1BLIT(mbytes,mbpr,mx,my,
							rast,x + w->behind.x - rast->x,osy - rast->y,
							xmax - x,dheight, oncolor );
				if(nexty >= ymax)
					return;
				break; /* break for nextx loop */
			}
			ydots = w->ydots[nextx]; /* get next ydots for nextx and rastid */
			if(rastid == ydots[y]) /* if same continue and check next one */
				continue;
			rast = w->rasts[rastid];
			rastid = ydots[y];
			MASK1BLIT(mbytes,mbpr,mx,my,rast,
					   x + w->behind.x - rast->x,osy - rast->y,
					   nextx - x,dheight, oncolor );
			mx += nextx - x;
			x = nextx;
		}
		x = firstx;
		mx = firstmx;
		my += nexty - y;
		y = nexty;
	}

}

static void
_win_mask2blit(UBYTE *mbytes, unsigned int mbpr, Coor mx, Coor my,
		Raster *wndo, Coor x, Coor y, Ucoor width, Ucoor height,
		Pixel oncolor, Pixel offcolor)
{
Wndo *w = (Wndo *)wndo;
SHORT xmax, ymax;
SHORT nextx, nexty;
LONG firstx;
LONG firstmx;
LONG dheight;
SHORT osy;
UBYTE *ydots;
int rastid;
Raster *rast;

	xmax = x + width;
	ymax = y + height;
	firstx = x;
	firstmx = mx;

	for(;;)
	{
		osy = y + w->behind.y;
		if((nexty = w->vchanges[y]) > ymax)
			dheight = ymax - y;
		else
			dheight = nexty - y;
		ydots = w->ydots[x];
		rastid = ydots[y];

		for(;;)
		{
			nextx = NEXTX(ydots);
			if(nextx >= xmax)
			{
				rast = w->rasts[rastid];
				MASK2BLIT(mbytes,mbpr,mx,my,
						   rast,x + w->behind.x - rast->x,osy - rast->y, 
						   xmax - x,dheight, oncolor, offcolor );
				if(nexty >= ymax)
					return;
				break; /* break for nextx loop */
			}
			ydots = w->ydots[nextx]; /* get next ydots for nextx and rastid */
			if(rastid == ydots[y]) /* if same continue and check next one */
				continue;
			rast = w->rasts[rastid];
			rastid = ydots[y];
			MASK2BLIT(mbytes,mbpr,mx,my,
					   rast,x + w->behind.x - rast->x,osy - rast->y, 
					   nextx - x,dheight, oncolor, offcolor );
			mx += nextx - x;
			x = nextx;
		}
		x = firstx;
		mx = firstmx;
		my += nexty - y;
		y = nexty;
	}

}
#ifdef SLUFFED
static check_set_mcalt(Wscreen *s,LONG start,LONG count, Rgb3 *ctab)
{
Rgb3 *ideals;
UBYTE mask;

	if((count += start) <= FIRST_MUCOLOR)
		return;

	if(start < FIRST_MUCOLOR)
	{
		ctab += FIRST_MUCOLOR - start;
		start = FIRST_MUCOLOR;
		ideals = s->mc_ideals;
		mask = 1;
	}
	else
	{
		ideals = &(s->mc_ideals[start - FIRST_MUCOLOR]);
		mask = 1 << (start - FIRST_MUCOLOR); 
	}

	while(start < count)
	{
		if(mask & s->mc_alt)
		{
			if(ideals->r != ctab->r
				|| ideals->g != ctab->g
				|| ideals->b != ctab->b)
			{
				s->mc_alt &= ~mask;
			}
		}
		++ideals;
		++ctab;
		mask <<= 1;
	}
}
#endif /* SLUFFED */
static void win_set_colors(Raster *wndo, LONG start, LONG count, void *ctab)
{
Wndo *w = (Wndo *)wndo;
Wscreen *s = w->W_screen;

	SET_COLORS(s->viscel,start,count,ctab);
	if(s->flags & WS_MUCOLORS_UP)
	{
#ifdef NOTNEEDED
		if(s->mc_alt)
			check_set_mcalt(s, count, start, ctab);
#endif /* NOTNEEDED */
		set_refresh(s);
	}
}
#ifdef SLUFFED
static void cset_check_mcalt(Wscreen *s)
{
register Rgb3 *ideals;
register Rgb3 *altnew;
UBYTE mask;

	ideals = &(s->mc_ideals[NUM_MUCOLORS - 1]);
	altnew = &(s->viscel->cmap->ctab[FIRST_MUCOLOR + NUM_MUCOLORS - 1]);

	mask = 1 << (NUM_MUCOLORS - 1);
	while(mask)
	{
		if(mask & s->mc_alt)
		{
			if(altnew->r != ideals->r
				|| altnew->g != ideals->g
				|| altnew->b != ideals->b)
			{
				s->mc_alt &= ~mask;
			}
			mask >>= 1;
			--altnew;
			--ideals;
		}
	}
}
#endif /* SLUFFED */
static void win_uncc64(Raster *wndo, UBYTE *ctab)
{
Wndo *w = (Wndo *)wndo;
Wscreen *s = w->W_screen;

	UNCC64(s->viscel,ctab);
	if(s->flags & WS_MUCOLORS_UP)
		set_refresh(s);
}
static void win_uncc256(Raster *wndo, UBYTE *ctab)
{
Wndo *w = (Wndo *)wndo;
Wscreen *s = w->W_screen;

	UNCC256(s->viscel,ctab);
	if(s->flags & WS_MUCOLORS_UP)
		set_refresh(s);
}
static void win_wait_vsync(Raster *wndo)
{
	Wndo *w = (Wndo *)wndo;

	if(!(w->flags & (WNDO_HIDDEN)))
		WAIT_VSYNC(w->W_screen->viscel);
}
Rastlib *get_window_lib(void)
{
static int loaded = 0;
static Rastlib winlib;

	if(!loaded)
	{
		loaded = 1;
		winlib.cput_dot = win_put_dot;
		winlib.put_dot = _win_put_dot;
		winlib.cget_dot = win_get_dot;
		winlib.get_dot = _win_get_dot;
		winlib.put_hseg = _win_put_hseg;
		winlib.get_hseg = _win_get_hseg;
		winlib.put_vseg = _win_put_vseg;
		winlib.get_vseg = _win_get_vseg;
		winlib.set_hline = _win_set_hline;
		winlib.set_vline = _win_set_vline;
		winlib.set_rect = _win_set_rect;

		/* winlib.set_rast = NULL; let go through set rect since a wndo
		 * is a rect */

		winlib.xor_rect = _win_xor_rect;
		winlib.mask1blit = _win_mask1blit;
		winlib.mask2blit = _win_mask2blit;
		winlib.set_colors = win_set_colors;
		winlib.uncc64 = win_uncc64;
		winlib.uncc256 = win_uncc256;
		winlib.wait_vsync = win_wait_vsync;
		pj_set_grc_calls(&winlib);
	}
	return(&winlib);
}
