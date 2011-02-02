

/* seemenu.c - Visual part of Flicmenu interpreter.  Lots of
   "seeme" routines suitable for stuffing into Flicmenu structures.
   Recursive button display stuff is in here.  Other simple seeme's
   are in seeme.c */
#include "jimk.h"
#include "flicmenu.h"



ccolor_box(m)
Flicmenu *m;
{
mb_isquare(m,vs.ccolor);
mb_ccorner(m,sgrey);
}

#ifdef SLUFFED
see_menu_back( m)
Flicmenu *m;
{
int *p, x;

m2color_block(m, swhite, hilit(m));
if ((p = m->text)!=NULL)
	{
	while ((x = *p++) >= 0)
		colrop(swhite, m->x+x, m->y+1, 0, m->height-2);
	}
}
#endif SLUFFED

color_block1(color,m)
WORD color;
Flicmenu *m;
{
colrop(color,m->x+1, m->y+1, m->width-2, m->height-2);
}

m2color_block(m, fcolor, bcolor)
Flicmenu *m;
WORD fcolor, bcolor;
{
a_frame(fcolor, m);
color_block1(bcolor, m);
}


white_block(m)
Flicmenu *m;
{
a_block(swhite, m);
}

black_block(m)
Flicmenu *m;
{
a_block(sblack, m);
}

grey_block(m)
Flicmenu *m;
{
a_block(sgrey, m);
}


#ifdef SLUFFED
bwtext(m)
Flicmenu *m;
{
register char *string = m->text;

a_block(swhite, m);
if (string)
	gtext( string, 
		m->x + (1+m->width)/2 - string_width(string)/2+1, 
		m->y + (1+m->height)/2 - CH_HEIGHT/2, 
		hilit(m));
}
#endif SLUFFED

#ifdef SLUFFED
wbtext(m)
Flicmenu *m;
{
wbtext_offset(m, 0, 0);
}
#endif SLUFFED


#ifdef SLUFFED
wbtexty1(m)
Flicmenu *m;
{
wbtext_offset(m, 0, 1);
}
#endif SLUFFED

#ifdef SLUFFED
wbtext_offset(m, dx, dy)
Flicmenu *m;
WORD dx, dy;
{
char *string = m->text;

a_block(hilit(m), m);
gtext( string, 
	dx + m->x + (1+m->width)/2 - string_width(string)/2, 
	dy + m->y + (1+m->height)/2 - CH_HEIGHT/2 - 1, 
	swhite);
}
#endif SLUFFED


see_num(m, xoff, yoff, fore)
register Flicmenu *m;
WORD xoff, yoff, fore;
{
char buf[10];

sprintf(buf, "%d", *((WORD *)m->text));
gtext( buf, 
	xoff + m->x + (1+m->width)/2 - string_width(buf)/2, 
	yoff + m->y + (1+m->height)/2 - CH_HEIGHT/2 - 1, 
	fore);
}

menu_cursor(m, color,c)
Flicmenu *m;
int color;
struct cursor *c;
{
a1blit(c->width, c->height, 0, 0, c->image, Mask_line(c->width),
	m->x + (1+m->width)/2 - c->xhot, 
	m->y + (1+m->height)/2 - c->yhot, 
	vf.p, vf.bpr, color);
}


bcursor(m)
Flicmenu *m;
{
menu_cursor(m, hilit(m), m->text);
}


#ifdef SLUFFED
inverse_c(m, c)
Flicmenu *m;
struct cursor *c;
{
a_block(hilit(m), m);
menu_cursor(m, swhite,c);
}
#endif SLUFFED



#ifdef SLUFFED
extern struct cursor cleft, cright, cup, cdown, ctrileft, ctriright;


right_arrow(m)
Flicmenu *m;
{
inverse_c(m, &cright);
}
#endif SLUFFED

hilight(m)
Flicmenu *m;
{
WORD *save_group;

if (m->seeme != NULL)
	{
	save_group = m->group;
	m->group = &m->identity;
	draw_sel(m);
	m->group = save_group;
	}
}

draw_sel(m)
Flicmenu *m;
{
if (m != NULL && m->seeme != NULL)
	{
	(*m->seeme)(m);
	}
}




static
s_colors(m, lookup, divx)
Flicmenu *m;
UBYTE *lookup;
int divx;
{
WORD i, j, count;
WORD lastx, lasty;
WORD nextx, nexty;
WORD x, y;
WORD width, height;
WORD tx,ty;
int col;
int outcolor;


outcolor = hilit(m);
if (outcolor == sblack)
	outcolor = sgrey;
a_frame(outcolor, m);
x = m->x+1;
y = m->y+1;
width = m->width-2;
height = m->height-2;
count = 0;
lasty = y-1;
nexty = height + y;
lastx = x-1;
for (j=0; j<divx; )
	{
	nextx = (j+1)*width/divx + x;
	col = lookup[count];
	colblock(col, lastx+1, lasty+1, nextx, nexty);
	if (col == vs.ccolor)
		{
		tx = ((nextx+lastx)>>1)+1;
		ty = (nexty+lasty)>>1;
		putdot(tx,ty,sbright);
		putdot(tx,ty+1,sblack);
		}
	count+=1;
	lastx = nextx;
	j++;
	}
lasty = nexty;
}
extern WORD circ2_cursor[];

static
f_colors(m, lookup, divx)
Flicmenu *m;
UBYTE *lookup;
int divx;
{
WORD j, count;
WORD nextx;
WORD x;
WORD width;


x = m->x+1;
width = m->width-2;
count = 0;
for (j=0; j<divx-1; )
	{
	nextx = (j+1)*width/divx + x;
	if (uzx <= nextx)
		return(lookup[count]);
	count+=1;
	j++;
	}
return(lookup[count]);
}


see_cluster(m)
Flicmenu *m;
{
struct bundle *bun;

bun = vs.buns + m->identity;
s_colors(m, bun->bundle, bun->bun_count);
}

f_cbun(m)
Flicmenu *m;
{
struct bundle *bun;

bun = vs.buns + m->identity;
return(f_colors(m, bun->bundle, bun->bun_count));
}


feel_cluster(m)
Flicmenu *m;
{
vs.ccolor = f_cbun(m);
vs.cycle_draw = 0;
predraw();
}



#ifdef SLUFFED
Flicmenu *
parent_menu(parent, match)
Flicmenu *parent, *match;
{
Flicmenu *m;
Flicmenu *temp;

m = parent->children;
while (m)
	{
	if (m == match)
		return(parent);
	temp = parent_menu(m, match);
	if (temp != NULL)
		return(temp);
	m = m->next;
	}
return(NULL);
}
#endif SLUFFED


draw_menus(m)
register Flicmenu *m;
{
if (m->seeme)
	(*m->seeme)(m);
if (m->children)
	draw_menus(m->children);
if (m->next)
	draw_menus(m->next);
}

static WORD *group;
static char change;
static WORD saveid;

static
uh_group(m)
register Flicmenu *m;
{

if (m->group == group)
	{
	if (*m->group == m->identity)
		{
		saveid = m->identity;
		if (change)
			m->identity = !m->identity;
		draw_sel(m);
		m->identity = saveid;
		}
	}
if (m->children)
	uh_group(m->children);
if (m->next)
	uh_group(m->next);
}

unhi_group(m, mgroup)
register Flicmenu *m;
WORD *mgroup;
{
group = mgroup;
change = 1;
uh_group(m);
}

hi_group(m, mgroup)
register Flicmenu *m;
WORD *mgroup;
{
group = mgroup;
change = 0;
uh_group(m);
}


#ifdef SLUFFED
cursor_box(m)
Flicmenu *m;
{
inverse_c(m, m->text);
a_frame(swhite, m);
}
#endif SLUFFED

#ifdef SLUFFED
text_boxp1(m)
Flicmenu *m;
{
a_block(hilit(m), m);
a_frame(swhite, m);
gtext( m->text, 
	m->x + (1+m->width)/2 - string_width(m->text)/2 + 2, 
	m->y + (1+m->height)/2 - CH_HEIGHT/2, 
	swhite);
}
#endif SLUFFED

#ifdef SLUFFED
white_frame(m)
Flicmenu *m;
{
a_frame(swhite, m);
}
#endif SLUFFED


