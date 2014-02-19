#include <string.h>
#include "errcodes.h"
#include "memory.h"
#define WNDO_INTERNALS
#include "wndo.h"
#include "ptrmacro.h"


#ifdef DEBUG
#define pj_stuff_pointers(d,b,np,num) \
	{ if((np) > 0) \
		pj_stuff_pointers(d,b,(np)); \
	else { \
		boxf("stuff pointers %d #%d", (np), num);\
		pj_stuff_pointers(d,b,-(np));}}
#else
	#define pj_stuff_pointers(d,b,np,num) pj_stuff_pointers(d,b,np)
#endif /* DEBUG */


typedef struct clipwork {
	Cliprect rect;		  /* clip rectangle from root window port */
	UBYTE **ydots;		  /* the array from the root window */
	Cliphead **freeclips;
} Clipwork;

static void merge_crect_ydots(Clipwork *cw, Cliprect *crect,SHORT id)

/* merges a rectangle into a ydots array it is assumed this rectangle is
 * clipped already before this is called */
{
UBYTE *dbuf;	/* current dotbuf */
UBYTE *nextdbuf; /* next one we are building or getting*/
UBYTE **dbufs;	/* the array of them indexed by y relative to dw.rect.y */
Rectangle osrect; /* offset rectangle */
SHORT wdif;
SHORT oymax;	  /* offset xmax of cliprect */
SHORT hbelow;	  /* height below cliprect in dw->rect */

#ifdef DEBUG
	boxf("mrg x %d y %d mx %d my %d", crect->x, crect->y, crect->MaxX,
									 crect->MaxY );

	boxf("into x %d y %d mx %d my %d", cw->rect.x, cw->rect.x,
									   cw->rect.MaxX, cw->rect.MaxY );
#endif

	/* build offset rect here so its done only once */

	crect_torect(crect,&osrect);
	osrect.x -= cw->rect.x;
	osrect.x -= cw->rect.y;
	oymax = osrect.y + osrect.height;
	hbelow = cw->rect.MaxY - crect->MaxY;

	dbufs = cw->ydots;
	dbuf = dbufs[osrect.x]; /* get pointer to dotbuf for left edge */

	if(STARTX(dbuf) < crect->x)
	{
	/* truncate previous range and put a dupe of it in the following range */
		nextdbuf = get_free_clip(cw->freeclips);
		NEXTX(nextdbuf) = NEXTX(dbuf); /* new one ends where last one did */
		/* last one ends and new one starts at rect start */
		NEXTX(dbuf) = STARTX(nextdbuf) = crect->x;
		pj_stuff_pointers(nextdbuf,&dbufs[osrect.x],
						NEXTX(nextdbuf) - STARTX(nextdbuf),0);

		if(NEXTX(nextdbuf) <= crect->MaxX)
		{
			/* copy ends, middle stuffed below */
			if(osrect.y)
				pj_copy_bytes(dbuf,nextdbuf,osrect.y);
			if(hbelow)
				pj_copy_bytes(dbuf + oymax, nextdbuf + oymax, hbelow);
			dbuf = nextdbuf;
			goto next_is_within; /* bypass while test below */
		}
		else
		{
			/* copy whole thing. it's used below */
			pj_copy_bytes(dbuf,nextdbuf,oymax + hbelow);
			dbuf = nextdbuf;
			goto next_is_beyond; /* bypass while test below */
		}
	}

	while(NEXTX(dbuf) <= crect->MaxX)
	{
next_is_within:
		/* current range is entirely within rect so set id bytes for x range */
		pj_stuff_bytes(id,dbuf + osrect.y, osrect.height);
		wdif = NEXTX(dbuf) - STARTX(dbuf);

		for(;;)
		{
			if(!(osrect.width -= wdif)) /* last one finished it done!! */
				return;
			osrect.x += wdif;

			nextdbuf = dbufs[osrect.x];

			/* if the merged current buffer and the merged next one
			 * will not be the the same we must keep both
			 * If they will be the same
			 * extent the current one to the others next
			 * or the rects end whichever is less
			 * and free the next one if not needed any more */

			if(memcmp(dbuf,nextdbuf,osrect.y)
				|| (hbelow && memcmp(dbuf + oymax, nextdbuf + oymax, hbelow)))
			{
				/* the two ydot segments are different after merging */
				dbuf = nextdbuf; /* keep next one */
				break; /* break for loop and merge to it */
			}
			if(NEXTX(nextdbuf) <= crect->MaxX) /* still in but same */
			{
				wdif = NEXTX(nextdbuf) - STARTX(nextdbuf);
				NEXTX(dbuf) = NEXTX(nextdbuf);
				pj_stuff_pointers(dbuf,&dbufs[osrect.x],wdif,1);
				add_free_clip(cw->freeclips,nextdbuf);
			}
			else /* next goes beyond rect shift border to rect end */
			{
				pj_stuff_pointers(dbuf,&dbufs[osrect.x],
								crect->MaxX - STARTX(nextdbuf),2);
				STARTX(nextdbuf) = NEXTX(dbuf) = crect->MaxX; /* shift right */
				return; /* in this case the rectangle is done !! */
			}
		}
	}

next_is_beyond:

	/* If we get here the last yseg is ending beyond the rect
	 * so we chop off the first part of its range and substitute a new one */

	nextdbuf = get_free_clip(cw->freeclips);
	STARTX(nextdbuf) = STARTX(dbuf); /* new one starts at old start */
	NEXTX(nextdbuf) = crect->MaxX; /* new one ends at rect end */
	STARTX(dbuf) = crect->MaxX; /* the old one now starts lower down */
	if(osrect.y)
		pj_copy_bytes(dbuf,nextdbuf,osrect.y); /* copy in beginning */
	pj_stuff_bytes(id,nextdbuf + osrect.y, osrect.height); /* stuff window id */
	if(hbelow)
		pj_copy_bytes(dbuf + oymax, nextdbuf + oymax, hbelow); /* copy in end */
	pj_stuff_pointers(nextdbuf,&dbufs[osrect.x],osrect.width,3);
	return;
}

void free_init_ydots(Wndo *w)
{
UBYTE *ydots;
SHORT nextx;
UBYTE rastid;

	nextx = 0;
	for(;;)
	{
		ydots = w->ydots[nextx];
		nextx = NEXTX(ydots);
		if(nextx >= w->behind.width)
			break;
		add_free_clip(&w->free_ydots,ydots);
	}
	/* install one ydots buffer for whole window which directs input
	 * to it's own raster (root is special in that it's set to screen
	 * and others are merged in to it) */

	if(w->type == RT_ROOTWNDO)
		rastid = SCREEN_RASTID;
	else
		rastid = w->W_rastid;

	pj_stuff_bytes(rastid,ydots,w->behind.height);

	STARTX(ydots) = 0;
	NEXTX(ydots) = w->behind.width;
	pj_stuff_pointers(ydots,w->ydots,w->behind.width,4);
}

static void load_root_backdots(Wndo *root, Wndo *w, Cliprect *Union)

/* copys ydots buffers from root into window that are in "Union" area around
 * Union is pointed to windows own backup area */
{
UBYTE **wdbufs; /* window ydbufs pointers */
UBYTE **rdbufs; /* root ydbufs pointers adjusted for relative offset */
SHORT wx;	  /* window relative x of Union */
SHORT wy;	  /* window relative y of Union */
SHORT ry;	  /* root relative y of Union */
SHORT width;  /* the Union width */
SHORT height; /* the Union height */
SHORT dwidth; /* segment width */
UBYTE *wydots; /* curent window ydots array */
UBYTE *rydots; /* current root ydots array */
SHORT wMaxY;	/* end of Union in window */
SHORT behind_below; /* right part behind if window is smaller than Union */
SHORT wxoset;		/* window x offset relative to root */

	wdbufs = w->ydots;	/* no offset */
	wxoset = (w->behind.x - root->x);
	rdbufs = root->ydots + wxoset; /* adjust for xoffset of wx = 0 */

	wx = Union->x;
	width = Union->MaxX - wx;
	wx -= w->behind.x;

	ry = wy = Union->y;
	height = Union->MaxY - wy;
	wy -= w->behind.y;
	ry -= root->y;
	wMaxY = wy + height;
	behind_below = w->behind.height - wMaxY;

	wydots = wdbufs[0]; /* assume it is initialized with one list */

	if(wx != 0) /* Union is below top of window raster truncate top */
	{
		NEXTX(wydots) = wx;
		wydots = get_free_clip(&w->free_ydots);
		STARTX(wydots) = wx;
	}

	for(;;)
	{
		rydots = rdbufs[wx];
		dwidth = (NEXTX(rydots) - wxoset) - wx; /* get range of root ydots */
		if(dwidth > width) /* clip if bigger */
			dwidth = width;
		if(wy)
			pj_stuff_bytes(w->W_rastid,wydots,wy);
		pj_copy_bytes(rydots + ry,wydots + wy,height); /* copy in root dots */
		if(behind_below)
			pj_stuff_bytes(w->W_rastid,wydots + wMaxY,behind_below);
		pj_stuff_pointers(wydots,&wdbufs[wx],dwidth,5);  /* load pointers */
		wx += dwidth;		 /* get next wx */
		NEXTX(wydots) = wx;  /* new next start load previously */
		width -= dwidth;
		if(width == 0)
			break;
		wydots = get_free_clip(&w->free_ydots);
		STARTX(wydots) = wx;
	}
	if(wx == w->behind.width) /* done */
		return;

	/* some left covered on right */
	wydots = get_free_clip(&w->free_ydots);
	STARTX(wydots) = wx;
	NEXTX(wydots) = w->behind.width;
	pj_stuff_bytes(w->W_rastid,wydots,w->behind.height);
	/* load pointers */
	pj_stuff_pointers(wydots,&wdbufs[wx],w->behind.width - wx,6);
	return;
}
static Boolean isvchange(SHORT x,SHORT y,UBYTE **ydots,SHORT width)

/* scans to see if there really is a vertical change at the window y given
 * by scanning ydots array */
{
register UBYTE *dots;

	for(;;)
	{
		if(x >= width)
			return(0);
		dots = ydots[x];
		if(dots[y] != dots[y-1])
			return(1);
		x = NEXTX(dots);
	}
}
static void load_vchanges(SHORT *vchanges,Wndo *w,Cliprect *clip)

/* note: this can be used when vchanges is the windows vchanges */
{
SHORT nextvchange;
SHORT winy, wintop;
SHORT *vcbuf;

	vcbuf = w->vchanges;

	if(w->behind.y + w->behind.height > clip->MaxY)
	{
		winy = clip->MaxY - w->behind.y;
		pj_stuff_words(w->behind.height,&vcbuf[winy],w->behind.height - winy);
	}
	else
		winy = w->behind.height;

	if((wintop = w->behind.y - clip->y) < 0)
	{
		wintop = -wintop;
		pj_stuff_words(wintop,vcbuf,wintop);
	}

	nextvchange = winy;
	--winy;
	vcbuf += winy;
	vchanges += (w->behind.y + winy); /* offset to current window position */
	while(winy > wintop)
	{
		if(!(*vchanges--)) /* zero is set in rect clip loop */
		{
			*vcbuf-- = nextvchange; /* must be here incase vcbuf == vchanges */
			if(isvchange(0,winy,w->ydots,w->behind.width))
				nextvchange = winy;
		}
		else
			*vcbuf-- = nextvchange; /* must be here incase vcbuf == vchanges */
		--winy;
	}
	*vcbuf = nextvchange;
}
void build_all_clips(Wscreen *ws,USHORT full_update)

/* loads all ydots that point to backup areas or "backs" of windows above
 * arrays for windows from head to root including root it is also assumed that
 * all windows above root need only backup areas specified to backup root
 * and windows between root and that window allways called with root being
 * the screen window and the clipwork setup for root. it is assumed
 * The root has no offset between its "behind" and the window port
 * returns 1 if root window is obscured by a window 0 if not */
{
Clipwork cw;
Wndo *w;
Cliprect clipped;
SHORT *vchanges;
int root_obscured = 0;
USHORT inflags;
Wndo *root;

	root = &ws->wndo;
	cw.freeclips = &(root->free_ydots);
	cw.rect = *((Cliprect *)&(root->x));
	cw.ydots = root->ydots;

	/* it is assumed that there will always be a head because root is a member
	 * of root->W_screen->wilist (and must be or else it won't terminate !!) */

	free_init_ydots(root);
	vchanges = root->vchanges;
	pj_stuff_words(root->behind.height,vchanges,root->behind.height);
	vchanges -= root->y; /* adjust for root offset */
	inflags = root->flags;

	w = (Wndo *)see_head(&(root->W_screen->wilist));

	for(;;)
	{
		w = TOSTRUCT(Wndo,W_node,w);
		if(w == root)  /* all done !! */
			break;
		free_init_ydots(w);

		if( !(w->flags & WNDO_HIDDEN)
			&& and_cliprects((Cliprect *)&(cw.rect.CRECTSTART),
							  (Cliprect *)&(w->CRECTSTART),
							  &clipped))
		{
			load_root_backdots(root,w,&clipped);
			load_vchanges(vchanges,w,&clipped);

			if(!(w->flags & WNDO_BACKDROP))
			{
				if(clipped.y < cw.rect.MaxY)
					vchanges[clipped.y] = 0;
				if(clipped.MaxY < cw.rect.MaxY)
					vchanges[clipped.MaxY] = 0;
				merge_crect_ydots(&cw,&clipped,w->W_rastid);
				root_obscured = 1;
			}

			inflags |= w->flags;

			if( (NEXTX(w->ydots[0]) == w->behind.width)
				&&	(w->vchanges[0] == w->behind.height))
			{
				w->onerast = *(w->ydots[0]);
				if(w->rasts[w->onerast]->x != w->behind.x
					|| w->rasts[w->onerast]->y != w->behind.y)
				{
					w->lib = get_wndo_r1oslib(); /* offset */
				}
				else
					w->lib = get_wndo_r1lib(); /* no offset */
			}
			else
				w->lib = get_window_lib();
		}
		else /* no overlapwith root or hidden there are no vchanges */
		{
			pj_stuff_words(w->behind.height,w->vchanges,w->behind.height);
			w->onerast = w->W_rastid;
			w->lib = get_wndo_r1lib(); /* never offset to itself */
		}

		w = (Wndo *)(w->W_node.next);
	}
	rect_tocrect((Rectangle *)&root->behind.RECTSTART,&clipped);
	load_vchanges(vchanges,root,&clipped);

	if(root_obscured)
		root->lib = get_window_lib();
	else
		root->lib = get_wndo_r1lib(); /* root never offset onerast always 0 */

	if(full_update)
	{
		if(inflags & WNDO_MUCOLORS & ws->flags)
		{
			if(!(ws->flags & WS_MUCOLORS_UP) || lastmuc_changed(ws))
				find_mucolors(ws);
		}
		else
		{
			if(ws->flags & WS_MUCOLORS_UP)
				uncheck_mucmap(ws);
		}
	}
	return;
}
