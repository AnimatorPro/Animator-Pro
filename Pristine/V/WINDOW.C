
/* window.c - Direct pixel output around menus and behind Zoom screen.
   This is what makes rendering a solid box slow in Vpaint... */

#include "jimk.h"
#include "flicmenu.h"

/* structures for minimalist 'windowing' system that lets us draw behind
   menus.  Horizontal strips only.  */

struct wi_list
	{
	struct wi_list *next;
	int y0,y1;
	PLANEPTR image;
	};

struct dwindow
	{
	int y0,y1;
	PLANEPTR image;
	char inmenu, flags;
	};


extern WORD *abehind, *mbehind;
char anywin;

struct dwindow dwlist[8];
static struct dwindow zw[8];
static int dw_ct;
static int zw_ct;

UBYTE *dwlinestart[YMAX]; /* ycoordinate lookup table for drawing... */	
UBYTE menu_space[YMAX];		/* 1's where there's menus */

static struct wi_list wia[4] = {
	{wia+1,},
	{wia+2,},
	{wia+3,},
	{NULL,},
	};

static struct wi_list *free_wis = wia;
static struct wi_list *menu_wis;


in_control_space()
{
extern WORD uzy;

if (mouse_button & 0x80)
	return(1);
return(on_menus(uzy) );
}

#ifdef SUBROUTINE
on_menus(y)
int y;
{
register struct wi_list *a;


a = menu_wis;
while (a != NULL)
	{
	if (y >= a->y0 && y < a->y1)
		{
		return(1);
		}
	a = a->next;
	}
return(0);
}
#endif SUBROUTINE

static
new_dw(y0,y1,image,inmenu)
int y0,y1;
PLANEPTR image;
int inmenu;
{
register struct dwindow *d;

d = dwlist+dw_ct;
dw_ct++;
d->y0 = y0;
d->y1 = y1;
d->image = image;
d->inmenu = inmenu;
}

make_dw()
{
int lasty;
register struct wi_list *a;
int endy;
int y;

zero_structure(dwlist, sizeof(dwlist) );
lasty = 0;
a = menu_wis;
dw_ct = 0;
while (a)
	{
	y = a->y0;
	if (y > lasty)
		{
		new_dw(lasty, y, render_form->p+lasty*BPR, 0);
		}
	lasty = a->y1;
	new_dw(y, lasty, a->image, 1);
	a = a->next;
	}
if (lasty < YMAX)
	new_dw(lasty, YMAX, render_form->p+lasty*BPR, 0);
if (vs.zoom_mode)
	{
	copy_structure(dwlist,zw,sizeof(zw) );
	zw_ct = dw_ct;
	dwlist[0].y0 = 0;
	dwlist[0].y1 = YMAX;
	dwlist[0].image = render_form->p;
	dw_ct = 1;
	}
anywin = (dw_ct != 1);
make_lookups();
}


static
make_lookups()
{
register UBYTE **p = dwlinestart;
register UBYTE *image;
int i,y;

y = 0;
for (i=0; i<dw_ct; i++)
	{
	image = dwlist[i].image;
	while (y < dwlist[i].y1)
		{
		*p++ = image;
		image += BPR;
		y++;
		}
	}
stuff_bytes(0, menu_space, YMAX);
if (abehind != NULL)
	stuff_bytes(1, menu_space+cur_pull->yoff, cur_pull->height);
if (mbehind != NULL)
	stuff_bytes(1, menu_space+cur_menu->y, cur_menu->height+1);
}




UBYTE *dwlinestart[YMAX];

#ifdef SUBROUTINE
dwgetdot(x,y)
int x,y;
{
if (y >= dwlist[0].y0 && y < dwlist[0].y1)
	return(dwlist[0].image[(y-dwlist[0].y0)*BPR+x]);
else if (y >= dwlist[1].y0 && y < dwlist[1].y1)
	return(dwlist[1].image[(y-dwlist[1].y0)*BPR+x]);
else if (y >= dwlist[2].y0 && y < dwlist[2].y1)
	return(dwlist[2].image[(y-dwlist[2].y0)*BPR+x]);
else if (y >= dwlist[3].y0 && y < dwlist[3].y1)
	return(dwlist[3].image[(y-dwlist[3].y0)*BPR+x]);
}
#endif SUBROUTINE

#ifdef SUBROUTINE
dwdot(x,y,color)
int x,y,color;
{
if (!anywin)
	{
	dwlist[0].image[y*BPR+x] = color;
	return;
	}
if (y >= dwlist[0].y0 && y < dwlist[0].y1)
	dwlist[0].image[(y-dwlist[0].y0)*BPR+x] = color;
else if (y >= dwlist[1].y0 && y < dwlist[1].y1)
	dwlist[1].image[(y-dwlist[1].y0)*BPR+x] = color;
else if (y >= dwlist[2].y0 && y < dwlist[2].y1)
	dwlist[2].image[(y-dwlist[2].y0)*BPR+x] = color;
else if (y >= dwlist[3].y0 && y < dwlist[3].y1)
	dwlist[3].image[(y-dwlist[3].y0)*BPR+x] = color;
}
#endif SUBROUTINE



static
cmp_wis(w1,w2)
struct wi_list *w1, *w2;
{
return(w1->y0 - w2->y0);
}

static
add_wi(y0, y1, image)
int y0, y1;
PLANEPTR image;
{
register struct wi_list *new;

if ((new = free_wis) == NULL)
	{
#ifdef DEBUG
	continu_line("No free wis!");
#endif DEBUG
	return(0);
	}
free_wis = new->next;
new->next = menu_wis;
new->image = image;
new->y0 = y0;
new->y1 = y1;
menu_wis = sort_list((Name_list *)new, cmp_wis);
return(1);
}

#ifdef LATER
remove_wi(wi)
register struct wi_list *wi;
{
remove_el(menu_wis,wi);
wi->next = free_wis;
free_wis = wi;
}
#endif LATER

static
new_wi_list()
{
register struct wi_list *wi, *next;

next = menu_wis;
while ((wi = next) != NULL)
	{
	next = wi->next;
	wi->next = free_wis;
	free_wis = wi;
	}
menu_wis = NULL;
}

make_wi_list()
{
new_wi_list();
if (abehind != NULL)
	{
	add_wi(cur_pull->yoff, cur_pull->yoff+cur_pull->height, abehind);
	}
if (mbehind != NULL)
	{
	add_wi(cur_menu->y, cur_menu->y+cur_menu->height+1, mbehind);
	}
make_dw();
}

swap_undo()
{
swap_with_frame(&uf);
}

static
swap_with_frame(s)
Vscreen *s;
{
register int i;
PLANEPTR p;
unsigned len;

p = s->p;
for (i=0; i<dw_ct; i++)
	{
	len = (dwlist[i].y1 - dwlist[i].y0)*BPR;
	exchange_words(dwlist[i].image, p, len/2);
	p += len;
	}
exchange_words(render_form->cmap, s->cmap, COLORS/2*3);
}

save_undo()
{
copy_from_frame(&uf);
}

static
copy_from_frame(s)
Vscreen *s;
{
register int i;
PLANEPTR p;
unsigned len;

p = s->p;
for (i=0; i<dw_ct; i++)
	{
	len = (dwlist[i].y1 - dwlist[i].y0)*BPR;
	copy_structure(dwlist[i].image, p, len);
	p += len;
	}
copy_cmap(render_form->cmap, s->cmap);
}

static
copy_frame_screen(s)
Vscreen *s;
{
register int i;
PLANEPTR p;
unsigned len;

p = s->p;
for (i=0; i<dw_ct; i++)
	{
	len = (dwlist[i].y1 - dwlist[i].y0)*BPR;
	copy_structure(p, dwlist[i].image, len);
	p += len;
	}
copy_cmap(s->cmap, render_form->cmap);
}

unundo()
{
copy_frame_screen(&uf);
}

zoom_it()
{
register int i;

if (!vs.zoom_mode)
	return;
for (i=0; i<zw_ct; i++)
	{
	if (!zw[i].inmenu)
		{
		zoom_seg(dwlist[i].y0, dwlist[i].y1-dwlist[i].y0);
		}
	}
}

static
zoom_seg(y0, height)
int y0, height;
{
if (vs.zoom4)
	{
	zoom4blit(vs.zoomw,height,
		vs.zoomx,vs.zoomy+y0/vs.zoomscale,
		zoom_form->p,zoom_form->bpr,0,y0,vf.p,vf.bpr);
	}
else
	{
	zoomblit(vs.zoomw,height,
		vs.zoomx,vs.zoomy+y0/vs.zoomscale,
		zoom_form->p,zoom_form->bpr,0,y0,vf.p,vf.bpr);
	}
}

upd_zoom_dot(x,y,c)
int x,y,c;
{
register PLANEPTR p;
int i;

/* first make sure dot would be inside screen */
if ((x -= vs.zoomx) < 0)
	return;
if ((y -= vs.zoomy) < 0)
	return;
if (x >= vs.zoomw || y >= vs.zoomh)
	return;
if (vs.zoom4)
	{
	x <<= 2;
	y <<= 2;
	p = vf.p + y*BPR + x;
	if (!on_menus(y))
		{
		p[0] = c;
		p[1] = c;
		p[2] = c;
		p[3] = c;
		}
	y++;
	if (!on_menus(y))
		{
		p[0+1*BPR] = c;
		p[1+1*BPR] = c;
		p[2+1*BPR] = c;
		p[3+1*BPR] = c;
		}
	y++;
	if (!on_menus(y))
		{
		p[0+2*BPR] = c;
		p[1+2*BPR] = c;
		p[2+2*BPR] = c;
		p[3+2*BPR] = c;
		}
	y++;
	if (!on_menus(y))
		{
		p[0+3*BPR] = c;
		p[1+3*BPR] = c;
		p[2+3*BPR] = c;
		p[3+3*BPR] = c;
		}
	}
else
	{
	x <<= 1;
	y <<= 1;
	p = vf.p + y*BPR + x;
	if (!on_menus(y))
		{
		p[0] = c;
		p[1] = c;
		}
	if (!on_menus(y+1))
		{
		p[BPR] = c;
		p[BPR+1] = c;
		}
	}
}


restore_top_bar()
{
if (abehind != NULL)
	see_pull(0,0,cur_pull);
else
	{
	if (vs.zoom_mode)
		zoom_seg(0, root_pull.height);
	else
		blit8(XMAX, root_pull.height, 0, 0, uf.p,  uf.bpr, 
			0, 0, render_form->p, uf.bpr);
	}
}

#ifdef SLUFFED
blank_top_bar()
{
colrop(swhite, 0, 0, XMAX-1, root_pull.height-2);
}
#endif SLUFFED

ltop_text(s)
char *s;
{
int len;

if (vs.dcoor)
	{
	len = strlen(s) * CH_WIDTH;
	cblock(vf.p, 0, 0, len, 1, swhite);
	stext(s,0,0,sblack,swhite);
	return(len);
	}
}

top_text(s)
char *s;
{
int len;

len = ltop_text(s);
if (abehind != NULL)
	{
	if (vs.dcoor)
		cblock(vf.p, len, 1, XMAX-len, CH_HEIGHT-2, swhite);
	}
}

