
/* cel.c - Stuff to do the basic things you need to do to a cel (ie a
   rectangular image with a color map.  Vcels and Vscreens (aka
   screens) are almost the same structure.  You can use all the cel
   routines on a screen as well.  There's some interactive cel positioning
   and pasting routines here too.  Also some of the other easy stuff on
   the Cel drop-down */

#include "jimk.h"
#include "flicmenu.h"
#include "fli.h"
#include "inks.h"
#include "cel.str"

extern WORD x_0,y_0,x_1,y_1;
extern char under_flag;

/* Release a cel back onto the heap */
free_cel(c)
Vcel *c;
{
if (c != NULL)
	{
	gentle_freemem(c->cmap);
	gentle_freemem(c->p);
	freemem(c);
	}
}

/* Try to find room on the heap for a cel */
Vcel *
alloc_cel(w,h,x,y)
unsigned w,h,x,y;
{
Vcel *c;

if ((c = askmem(sizeof(*c))) == NULL)
	return(NULL);
if ((c->p = askmem(Raster_block(w, h))) == NULL)
	{
	freemem(c);
	return(NULL);
	}
if ((c->cmap = askmem(COLORS*3)) == NULL)
	{
	freemem(c->p);
	freemem(c);
	return(NULL);
	}
c->bpr = Raster_line(w);
c->w = w;
c->h = h;
c->x = x;
c->y = y;
return(c);
}

/* Make an identical instance of a cel. */
Vcel *
clone_cel(s)
Vcel *s;
{
Vcel *d;

if ((d = alloc_cel(s->w, s->h, s->x, s->y)) != NULL)
	{
	copy_cmap(s->cmap, d->cmap);
	copy_bytes(s->p, d->p, Raster_block(s->w, s->h) );
	}
return(d);
}

/* Display cel with marqi outline for a couple of jiffies. */
show_cel_a_sec()
{
int mod;

if (cel != NULL)
	{
	save_undo();
	see_cel();
	marqidata.c0 = swhite;
	marqidata.c1 = sblack;
	mod = 8;
	while (--mod >= 0)
		{
		marqidata.mod = mod;
		rub_cel_box(cel);
		wait_a_jiffy(4);
		}
	unsee_cel();
	}
}


/* Load up a cel from disk.  Allocate the memory for it as you go.
   Return pointer to it if successful, NULL if not. */
static Vcel *
ld_cel(name, ocel)
char *name;
Vcel *ocel;	/* Cel to free if looks like load OK, or NULL */
{
int file;
struct pic_header pic;
Vcel *c;
long size;

if ((file = jopen(name, 0)) == 0)
	{
	cant_find(name);
	return(0);
	}
if (jread(file, &pic, (long)sizeof(pic) ) < sizeof(pic) )
	{
	truncated(name);
	goto CLOSEOUT;
	}
if (pic.type != PIC_MAGIC)
	{
	continu_line(cel_100 /* "Not a CEL file" */);
	goto CLOSEOUT;
	}
free_cel(ocel);
if ((c = alloc_cel(pic.w,pic.h,pic.x,pic.y)) == NULL)
	{
	outta_memory();
	goto CLOSEOUT;
	}
if (jread(file, c->cmap, COLORS*3) < COLORS*3)
	{
	truncated(name);
	goto FREEOUT;
	}
size = (long)c->bpr*c->h;
if (jread(file,c->p,size) < size)
	{
	truncated(name);
	goto FREEOUT;
	}
jclose(file);
return(c);

FREEOUT:
free_cel(c);
CLOSEOUT:
jclose(file);
return(NULL);
}

/* Load not just any old cel, but THE cel */
load_cel(name)
char *name;
{
return((cel = ld_cel(name,cel)) != NULL);
}


/* Create a cel from a rectangular patch of the screen */
static 
Vcel *
clip_from_vf(w,h,x0,y0)
int w, h, x0, y0;
{
register Vcel *c;

if ((c = alloc_cel(w,h,x0,y0)) == NULL)
	{
	outta_memory();
	return(NULL);
	}
copy_structure(render_form->cmap, c->cmap, COLORS*3);
blit8(w, h, x0, y0, render_form->p, BPR,
	0, 0, c->p, c->bpr);
return(c);
}

/* Make a cel from rectangular area of screen user has defined with
   a box */
static
Vcel *
clip_from_rub_box()
{
return(clip_from_vf( intabs(grid_x-firstx)+1, intabs(grid_y-firsty)+1,
	intmin(grid_x, firstx), intmin(grid_y, firsty)));
}


/* place boundaries of area of screen not tcolor into global variables
   x_0, y_0, x_1, y_1 */
find_clip(screen,tcolor)
Vscreen *screen;
int tcolor;
{
register PLANEPTR c;
register WORD i;
register WORD temp;


y_0 = 0;
y_1 = screen->h;
c = screen->p;

/* figure out the first line in screen with anything in it*/
i = screen->h;
while (--i >= 0)
	{
	if (!(*c == tcolor && bsame(c, screen->bpr) == screen->bpr))
		break;
	c += screen->bpr;
	y_0++;
	}
if (y_0 == screen->h)
	{
	return(0);
	}

/* figure out the last line in screen with anything in it*/
/* c points to start of last line */
c = screen->p+(screen->bpr*(screen->h-1));
i = screen->h;
while (--i >= 0)
	{
	if (!(*c == tcolor && bsame(c, screen->bpr) == screen->bpr))
		break;
	c -= screen->bpr;
	--y_1;
	}

i = y_1 - y_0;	/* the horizontal slice of screen with anything in it */
x_0 = XMAX;
x_1 = 0;
while (--i >= 0)
	{
	if (*c != tcolor)
		temp = 0;
	else
		temp = bsame(c, screen->bpr);
	if (temp < x_0)
		x_0 = temp;
	temp = screen->w - back_scan(tcolor,c+screen->bpr, screen->bpr);
	if (temp > x_1)
		x_1 = temp;
	c -= screen->bpr;
	}
y_1 -= 1;
x_1 -= 1;
return(1);
}


/* Put the bits of screen not the key color into THE cel */
clip_cel()
{
if (find_clip(render_form, vs.inks[0]))
	{
	free_cel(cel);
	cel = clip_from_vf(x_1-x_0+1,y_1-y_0+1,x_0,y_0);
	show_cel_a_sec();
	}
}


/* Have use define a box and then grab a cel from that box.  Aka
   "get cel" */
dupe_cel()
{
save_undo();
if (cut_out())
	{
	free_cel(cel);
	cel = clip_from_rub_box();
	}
}

Vector cbvec = tblit8;	/* 'blit' function */
Vector cmvec = tmove8;	/* 'move' function */

/* figure out the move and blit vectors to go with whether the key color
   is clear or not */
set_zero_clear()
{
if (vs.zero_clear)
	{
	cbvec = tblit8;
	cmvec = tmove8;
	}
else
	{
	cbvec = cmvec = blit8;
	}
}

/* Plop down a cel on screen */
static
see_a_cel(cl)
register Vcel *cl;
{
set_zero_clear();
(*cbvec)(cl->w, cl->h, 0, 0, cl->p, cl->bpr, 
	cl->x, cl->y, render_form->p, BPR,vs.inks[0]);
zoom_it();
}

/* Plop down THE cel on screen */
static
see_cel()
{
see_a_cel(cel);
}

/* Erase cel image (presuming undo screen's been saved).  Heck,
   would erase almost anything! */
static
unsee_cel()
{
unundo();
zoom_it();
}

/* Display a marqi frame around a cel */
static
rub_cel_box(c)
Vcel *c;
{
marqi_frame(c->x, c->y, c->x+c->w-1, c->y+c->h-1);
}

cmaps_same(s1, s2)
UBYTE *s1, *s2;
{
return(bcompare(s1, s2, COLORS*3) == COLORS*3);
}

/* Find closest colors in this color map to cel's color map, and then
   remap cel's pixels appropriately. */
cfit_cel(c,dcmap)
Vcel *c;
PLANEPTR dcmap;
{
UBYTE ctable[COLORS];

fitting_cmap(c->cmap, dcmap, ctable);
xlat(ctable,c->p,c->bpr*c->h);
}

/* Figure out if we can avoid color fitting the cel 'cause won't
   make any visible difference */
need_fit_cel(c)
Vcel *c;
{
return(vs.fit_colors && 
	vs.draw_mode == I_OPAQUE && 
	!cmaps_same(c->cmap, render_form->cmap) );
}

Vcel *ccc;	/* a color fitted copy of cel.  */


/* Point ccc to color fitted cel. If THE cel is color fitted
   already, relax, just point ccc to cel.  Otherwise allocate
   a copy of THE cel, fit it, and point ccc to this. */
make_ccc()
{
if (!vs.fit_colors || cmaps_same(cel->cmap, render_form->cmap) )
	ccc = cel;
else 
	{
	if ((ccc = clone_cel(cel)) == NULL)
		return(0);
	cfit_cel(ccc,render_form->cmap);
	}
return(1);
}

/* Free ccc if it's not THE cel */
free_ccc()
{
if (ccc != cel)
	{
	free_cel(ccc);
	}
}


/* keep track of how far user moved the cel this time... */
static  WORD mp_ox,mp_oy,mp_nx,mp_ny;

/* auto vec for moving a cel in a straight line.  Ie paste multi */
static
paste1(ix,it,scale)
int ix,it,scale;
{
int ok;
int dx,dy;

ok = 0;
if (!load_temp_cel())
	return(0);
dx = mp_nx - mp_ox;
dy = mp_ny - mp_oy;
cel->x = mp_ox + itmult(dx,scale);
cel->y = mp_oy + itmult(dy,scale);
#ifdef LATER
if (need_fit_cel())
#endif LATER
if (cfit_rblit_cel(cel))
	ok = 1;
free_cel(cel);
cel = NULL;
return(ok);
}

/* This routine draws a cel in place of an old cel while minimizing
   visible flicker without double buffering.  The area of old cel not
   covered by new cel is taken from the undo buffer. */
static
redraw_cel(c, ox,oy,ow,oh)
Vcel *c;
WORD ox,oy,ow,oh;
{
int oend, nend, dif;

set_zero_clear();
if ((dif=c->x-ox) > 0)	/* revealed some strip to left */
	{
	blit8(dif, oh, ox, oy, uf.p, uf.bpr,
		ox, oy, render_form->p, BPR);
	}
if ((dif=c->y-oy) > 0)
if (oy < c->y) /* revealed something up top */
	{
	blit8(ow, dif, ox, oy, uf.p, uf.bpr,
		ox, oy, render_form->p, BPR);
	}
oend = ox + ow;
nend = c->x + c->w;
if ((dif = oend - nend) > 0)	/* revealed a little to the right */
	{
	blit8(dif, oh, nend, oy, uf.p, uf.bpr,
		nend, oy,  render_form->p, BPR);
	}
oend = oy + oh;
nend = c->y + c->h;
if ((dif = oend - nend) > 0)	/* revealed a little to the on bottom */
	{
	blit8(ow, dif, ox, nend, uf.p, uf.bpr,
		ox, nend,  render_form->p, BPR);
	}
rub_cel_box(c);
if (c->w > 2 && c->h > 2)
	(*cmvec)(c->w-2, c->h-2, 1, 1, c->p, c->bpr, 
		c->x+1, c->y+1, render_form->p, BPR, vs.inks[0], uf.p);
zoom_it();
}

/* Move the old cel while minimizing horrible screen flashing.  */
static
move_screen_cel(c, dx, dy)
Vcel *c;
WORD dx, dy;
{
WORD x, y;

x = c->x;
y = c->y;
c->x = x+dx;
c->y = y+dy;
redraw_cel(c, x,y,cel->w, cel->h);
}


/* Move or paste a cel. */
static
mp_cel(paste)
WORD paste;
{
WORD mod, lx, ly;

mod = 0;
if (cel == NULL)
	return(0);
if (!make_ccc())
	return(0);
save_undo();
see_a_cel(ccc);
zoom_it();
mp_ox = ccc->x;
mp_oy = ccc->y;

if (rub_in_place(ccc->x, ccc->y, ccc->x+ccc->w-1, ccc->y+ccc->h-1))
	{
	clickonly = 1;
	for (;;)
		{
		lx = grid_x;
		ly = grid_y;
		vsync_input(4);
		marqidata.mod = mod++;
		if (mouse_moved)
			{
			move_screen_cel(ccc, grid_x-lx, grid_y-ly);
			}
		else
			rub_cel_box(ccc);
		box_coors(ccc->x, ccc->y, mp_ox, mp_oy);
		if (PJSTDN)
			break;
		if (RJSTDN)
			{
			paste = 0;
			ccc->x = mp_ox;
			ccc->y = mp_oy;
			break;
			}
		}
	clickonly = 0;
	}
unsee_cel();
mp_nx = cel->x = ccc->x;
mp_ny = cel->y = ccc->y;
restore_top_bar();
free_ccc();
if (paste)
	{
	if (vs.multi)
		{
		push_inks();
		push_cel();
		uzauto(paste1);
		pop_cel();
		pop_inks();
		}
	else
		{
		cfit_rblit_cel(cel);
		dirties();
		}
	}
zoom_it();
return(1);
}

/* Interactively move cel, but don't paste it when done */
move_cel()
{
mp_cel(0);
}

/* Interactively move and then paste cel */
paste_cel()
{
mp_cel(1);
dirties();
}

/* Interactively move and then paste cel beneath key color */
upaste_cel()
{
under_flag = 1;
mp_cel(1);
under_flag = 0;
}


/* The old move tool.  User defines a box.  We pretend a piece of
   the screen (or undo buffer actually) is a cel and move it to a
   new place.  Fill in the place they grabbed cel from with key color,
   and plop cel down in new position.  

   This is only place in vpaint where cel->bpr != cel->w.  */
move_tool()
{
Vcel *ocel;
Vcel lcel;
int startx,starty;

if (!pti_input())
	return;
save_undo();
if (rub_box())
	{
	swap_box();
	hide_mp();
	ocel = cel;
	cel = &lcel;
	lcel.p = uf.p+x_0+y_0*BPR;
	lcel.cmap = uf.cmap;
	lcel.bpr = uf.bpr;
	lcel.w = x_1-x_0+1;
	lcel.h = y_1-y_0+1;
	startx = lcel.x = x_0;
	starty = lcel.y = y_0;
	move_cel();
	cblock(render_form->p,startx,starty,cel->w,cel->h,vs.inks[0]);
	see_cel();
	zoom_it();
	cel = ocel;
	draw_mp();
	}
}


/* Squoosh down cel so all pixels not clear become color */
static
mask_cel(c, color, clear)
Vcel *c;
UBYTE color, clear;
{
register UBYTE *p, a;
long l;

p = cel->p;
l = c->bpr*c->h;
while (--l >= 0)
	{
	if (*p == clear)
		p += 1;
	else
		*p++ = color;
	}
}

/* Go mask THE cel and then display result a second */
vmask_cel()
{
mask_cel(cel, vs.ccolor, vs.inks[0]);
copy_cmap(render_form->cmap, cel->cmap);
show_cel_a_sec();
}

