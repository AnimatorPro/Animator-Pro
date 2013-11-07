
/* seeme.c - routines to display menus.  A lot of these aren't actually
   used just for file requestor.  */

#include "jimk.h"
#include "flicmenu.h"
#include "gemfont.h"

extern WORD *mbehind;	/* pointer to what's behind menu */


blit_menu(m, pt)
Flicmenu *m;
WORD *pt;
{
blit8(m->width+1, m->height+1, m->x, m->y, vf.p, vf.bpr,
	0,0,pt,Raster_line(m->width+1) );
}

WORD *
sunder_menu(m)
Flicmenu *m;
{
WORD *pt;

if ((pt = paskmem(Raster_block(m->width+1, m->height+1))) == NULL)
	return(0);
blit_menu(m, pt);
return(pt);
}

sdraw_menu(m)
Flicmenu *m;
{
if ((mbehind = sunder_menu(m))!=NULL)
	draw_a_menu(m);
return(mbehind != NULL);
}


unblit_menu(m, pt)
Flicmenu *m;
WORD *pt;
{
blit8(m->width+1, m->height+1, 0, 0, pt, Raster_line(m->width+1),
	m->x, m->y, vf.p, vf.bpr);
}

runder_menu(m,pt)
Flicmenu *m;
WORD *pt;
{
unblit_menu(m, pt);
pfreemem(pt);
}

draw_a_menu(m)
struct flicmenu *m;
{
find_colors();
qdraw_a_menu(m);
}

qdraw_a_menu(m)
struct flicmenu *m;
{
Flicmenu *onext;

if (m != NULL)
	{
	onext = m->next;
	m->next = NULL;
	draw_menus(m);
	m->next = onext;
	}
}

#ifdef LATER
static hmpstack;	/* keeps track of nesting of draw_mp/hide_mp calls */

/* Draw menus and pulls in Video Paint.  Here just draws menu */
draw_mp()
{
if (hmpstack == 0)
	{
	if (cur_menu)
		sdraw_menu(cur_menu);
	}
hmpstack++;
}

hide_mp()
{
if (--hmpstack == 0)
	{
	if (cur_menu)
		{
		runder_menu(cur_menu, mbehind);
		mbehind = NULL;
		}
	}
}
#endif  /* LATER */


bright(m)
struct flicmenu *m;
{
if (m->group)
	if (*m->group == m->identity)
		return(sbright);
return(swhite);
}

hilit(m)
struct flicmenu *m;
{
if (m->disabled)
	return(sgrey);
if (m->group)
	if (*m->group == m->identity)
		return(sred);
return(sblack);
}

a_block(color, m)
WORD color;
struct flicmenu *m;
{
colrop(color, m->x, m->y, m->width, m->height);
}

a_frame(color, m)
WORD color;
struct flicmenu *m;
{
draw_frame(color,  m->x,  m->y,  m->x + m->width+1, m->y + m->height+1);
}

box_cut_corner(x0,y0,w,h,color)
int x0,y0,w,h;
int color;
{
chli(vf.p, x0+1, y0, w-2, color);
chli(vf.p, x0+1, y0+h-1, w-2, color);
cvli(vf.p, x0, y0+1, h-2, color);
cvli(vf.p, x0+w-1, y0+1, h-2, color);
}

box_diag_corner(x0,y0,w,h,color)
int x0,y0,w,h;
int color;
{
int x1,y1;

x1 = x0+w;
y1 = y0+h;
chli(vf.p, x0+2, y0, w-4, color);
chli(vf.p, x0+2, y1-1, w-4, color);
cvli(vf.p, x0, y0+2, h-4, color);
cvli(vf.p, x1-1, y0+2, h-4, color);
cdot(vf.p,x0+1,y0+1,color);
cdot(vf.p,x0+1,y1-2,color);
cdot(vf.p,x1-2,y0+1,color);
cdot(vf.p,x1-2,y1-2,color);
}

diag_inside(x0,y0,w,h,color)
int x0,y0,w,h;
int color;
{
chli(vf.p, x0+2, y0+1, w-4, color);
cblock(vf.p,x0+1, y0+2, w-2, h-4, color);
chli(vf.p, x0+2, y0+h-2, w-4, color);
}

ccorner_inside(x0,y0,w,h,color)
int x0,y0,w,h;
int color;
{
cblock(vf.p,x0+1, y0+1, w-2, h-2, color);
}

mb_ccorner(m,color)
Flicmenu *m;
int color;
{
box_cut_corner(m->x,m->y,m->width + 1,m->height + 1,color);
}

mb_isquare(m,color)
Flicmenu *m;
int color;
{
ccorner_inside(m->x,m->y,m->width+1,m->height+1,color);
}

mb_dcorner(m,color)
Flicmenu *m;
int color;
{
box_diag_corner(m->x,m->y,m->width+1,m->height+1,color);
}

mb_inside(m,color)
Flicmenu *m;
int color;
{
diag_inside(m->x,m->y,m->width+1,m->height+1,color);
}


blacktext(m)
Flicmenu *m;
{
menu_text(m,hilit(m));
}

#ifdef SLUFFED
greytext(m)
Flicmenu *m;
{
menu_text(m,sgrey);
}
#endif  /* SLUFFED */

menu_text(m,color)
Flicmenu *m;
int color;
{
char *string;

if ((string = m->text) != NULL)
	gtext( string, 
		m->x + (1+m->width)/2 - string_width(string)/2+1, 
		m->y + (1+m->height)/2 - CH_HEIGHT/2+1, 
		color);
}


ccorner_text(m)
Flicmenu *m;
{
char *string;

mb_isquare(m,bright(m));
mb_ccorner(m,sgrey);
menu_text(m, hilit(m) );
}

dcorner_text(m)
Flicmenu *m;
{
mb_inside(m,bright(m) );
mb_dcorner(m,sgrey);
menu_text(m, hilit(m) );
}

#ifdef SLUFFED
move_tab_text(m)
Flicmenu *m;
{
int x0,y0,x1,y1;

diag_inside(m->x-1, m->y-1, m->width+3, m->height+3, sgrey);
menu_text(m,sblack);
}
#endif  /* SLUFFED */

#ifdef SLUFFED
ncorner_text(m)
Flicmenu *m;
{
mb_isquare(m,bright(m));
a_frame(sgrey,m);
menu_text(m,hilit(m));
}
#endif  /* SLUFFED */

#ifdef SLUFFED
ncorner_int(m)
Flicmenu *m;
{
ncnum(m, 1);
}
#endif  /* SLUFFED */

#ifdef SLUFFED
ncorner_number(m)
Flicmenu *m;
{
ncnum(m, 0);
}
#endif  /* SLUFFED */

#ifdef SLUFFED
ncnum(m, offset)
Flicmenu *m;
int offset;
{
char buf[10];
char *otext;

sprintf(buf, "%d", offset + *((int *)(m->text)));
otext = m->text;
m->text = buf;
mb_inside(m,bright(m));
ncorner_text(m);
m->text = otext;
}
#endif  /* SLUFFED */

#ifdef SLUFFED
ncorner_cursor(m)
Flicmenu *m;
{
a_frame(sgrey,m);
mb_isquare(m,bright(m));
menu_cursor(m, hilit(m), m->text);
}
#endif  /* SLUFFED */


ccorner_cursor(m)
Flicmenu *m;
{
mb_ccorner(m,sgrey);
mb_isquare(m,bright(m));
menu_cursor(m, hilit(m), m->text);
}


gary_see_title(m)
Flicmenu *m;
{
a_block(sgrey, m);
menu_text(m,swhite);
}


gary_menu_back(m)
Flicmenu *m;
{
m2color_block(m, sgrey, swhite);
}


#ifdef SLUFFED
gbnumber(m)
Flicmenu *m;
{
a_block(sgrey, m);
see_num(m, 0, 2, hilit(m));
}
#endif  /* SLUFFED */

#ifdef SLUFFED
gbnumber_plus1(m)
Flicmenu *m;
{
int *p;

p = (int *)m->text;
*p += 1;
gbnumber(m);
*p -= 1;
}
#endif  /* SLUFFED */

#ifdef SLUFFED
left_text(m)
Flicmenu *m;
{
char *string;

a_block(bright(m), m);
if ((string = m->text) != NULL)
	gtext( string, 
		m->x + 4, 
		m->y + (1+m->height)/2 - CH_HEIGHT/2+1, 
		hilit(m));
}
#endif  /* SLUFFED */


