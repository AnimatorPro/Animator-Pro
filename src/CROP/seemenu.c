
/* seemenu.c - more seeme routines to display menu buttons */

#include "jimk.h"
#include "flicmenu.h"




color_block1(color,m)
WORD color;
Flicmenu *m;
{
colrop(color,m->x+1, m->y+1, m->width-2, m->height-2);
}

m2color_block(m, fcolor, bcolor)
struct flicmenu *m;
WORD fcolor, bcolor;
{
a_frame(fcolor, m);
color_block1(bcolor, m);
}


white_block(m)
struct flicmenu *m;
{
a_block(swhite, m);
}

black_block(m)
struct flicmenu *m;
{
a_block(sblack, m);
}

#ifdef SLUFFED
grey_block(m)
struct flicmenu *m;
{
a_block(sgrey, m);
}
#endif  /* SLUFFED */



#ifdef SLUFFED
see_num(m, xoff, yoff, fore)
register struct flicmenu *m;
WORD xoff, yoff, fore;
{
char buf[10];

sprintf(buf, "%d", *((WORD *)m->text));
gtext( buf, 
	xoff + m->x + (1+m->width)/2 - string_width(buf)/2, 
	yoff + m->y + (1+m->height)/2 - CH_HEIGHT/2 - 1, 
	fore);
}
#endif  /* SLUFFED */

menu_cursor(m, color,c)
struct flicmenu *m;
int color;
struct cursor *c;
{
a1blit(c->width, c->height, 0, 0, c->image, Mask_line(c->width),
	m->x + (1+m->width)/2 - c->xhot, 
	m->y + (1+m->height)/2 - c->yhot, 
	vf.p, vf.bpr, color);
}


#ifdef SLUFFED
bcursor(m)
Flicmenu *m;
{
menu_cursor(m, hilit(m), m->text);
}
#endif  /* SLUFFED */


hilight(m)
struct flicmenu *m;
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
struct flicmenu *m;
{
if (m != NULL && m->seeme != NULL)
	{
	(*m->seeme)(m);
	}
}






draw_menus(m)
register struct flicmenu *m;
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

uh_group(m)
register struct flicmenu *m;
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
register struct flicmenu *m;
WORD *mgroup;
{
group = mgroup;
change = 1;
uh_group(m);
}

hi_group(m, mgroup)
register struct flicmenu *m;
WORD *mgroup;
{
group = mgroup;
change = 0;
uh_group(m);
}


