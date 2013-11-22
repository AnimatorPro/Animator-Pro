
/* Feelmenu.c - input handling routines for Flicmenus.
   Individual feelme's and a few routines to process whole menus. */

#include "jimk.h"
#include "flicmenu.h"
#include <ctype.h>
#include "feelmenu.str"

extern Flicmenu quick_menu;

Flicmenu *cur_menu = &quick_menu;



hang_child(m)
Flicmenu *m;
{
Flicmenu *child;

if ((child = m->children) != NULL)
	rmove_menu(child, m->x - child->x, m->y - child->y);
}

clip_rmove_menu(m,dx,dy)
Flicmenu *m;
WORD dx,dy;
{
if (m->x + dx < 0)
	dx = 0 - m->x;
if (m->y + dy < 0)
	dy = 0 - m->y;
if (m->x + m->width + 1 + dx >= XMAX)
	dx = XMAX - m->x - m->width - 1;
if (m->y + m->height + 1 + dy >= YMAX)
	dy = YMAX - m->y - m->height - 1;
rmove_menu(m, dx, dy);
}

rmove_menu(m,dx,dy)
Flicmenu *m;
WORD dx,dy;
{
if (!m)
	return;
rmove_menu(m->children, dx, dy);
rmove_menu(m->next, dx, dy);
m->x += dx;
m->y += dy;
}

bottom_menu(m)
Flicmenu *m;
{
hide_mp();
rmove_menu(cur_menu, vf.w-1 - cur_menu->width - cur_menu->x,
	vf.h-1-cur_menu->height - cur_menu->y);
draw_mp();
}

move_menu(m)
Flicmenu *m;
{
WORD y, my, ymin;
WORD *behind;
int mod,i;

hide_mp();
unzoom();
mod = 0;
m = cur_menu;
my = m->y;
behind = sunder_menu(m);
y = m->y;
marqi_frame(m->x, y, m->x+m->width, y+m->height);
for (;;)
	{
	vsync_input(4);
	if (PJSTDN || RJSTDN)
		break;
	if (RJSTDN)
		break;
	if (uzy != lasty)
		{
		y = m->y + uzy-lasty;
		ymin = (cur_pull ? cur_pull->yoff+cur_pull->height : 0);
		if (y < ymin)	/* end of top pull */
			y = ymin;
		if (y + m->height >= vf.h)
			y = vf.h - m->height - 1;
		runder_menu(m,behind);
		m->y = y;
		behind = sunder_menu(m);
		}
	marqidata.mod = mod++;
	marqi_frame(m->x, y, m->x+m->width, y+m->height);
	}
runder_menu(m,behind);
y = m->y - my;
m->y = my;
if (PJSTDN)
	rmove_menu(m,0,y);
rezoom();
draw_mp();
}

static
ptmout(timeout)
WORD timeout;
{
long time1;

time1 = get80hz() + timeout;
for (;;)
	{
	vsync_input((int)(time1-get80hz()));
	if (!PDN)
		return(0);
	if (get80hz() >= time1)
		return(1);
	}
}

static
pdn_timeout(timeout)
WORD timeout;
{
int ok;

ok = ptmout(timeout);
macrosync();
return(ok);
}

repeat_on_pdn(v)
Vector v;
{
WORD i;

(*v)();
if (!pdn_timeout(40))
	return;
i = 0;
for (;;)
	{
	(*v)();
	if (!pdn_timeout( i<10 ? 10 : 5) )
		return;
	i++;
	}
}

#ifdef SLUFFED
find_grid(m)
register Flicmenu *m;
{
register struct grid *g;
WORD x, y;
WORD width, height;

g = (struct grid *)m->text;
x = uzx - m->x - 1;
y = uzy - m->y - 1;
width = m->width-1;
height = m->height-1;
if (x < 0 || x >= width || y < 0 || y >= height )
	return(-1);

return(uscale_by(y, g->divy, height)*g->divx + uscale_by(x, g->divx, width) );
}
#endif SLUFFED

#ifdef SLUFFED
in_lr_half(m)
Flicmenu *m;
{
WORD xcenter;

xcenter = m->x + m->width/2;
if (uzx <= xcenter)
	return(-1);
else
	return(1);
}
#endif /* SLUFFED */

#ifdef SLUFFED
clip_slider(s)
struct slidepot *s;
{
if (s->value < s->min)
	s->value = s->min;
if (s->value > s->max)
	s->value = s->max;
}
#endif SLUFFED

#ifdef SLUFFED
slide_where(m)
Flicmenu *m;
{
struct slidepot *s;
WORD width;
WORD x;

s = (struct slidepot *)m->text;
x = uzx - m->x - 1;
width = m->width-2;
if (x < 0)
	return(s->min);
if (x >= width)
	return(s->max);
return(uscale_by(x, s->max - s->min + 1, width) + s->min);
}
#endif SLUFFED

#ifdef SLUFFED
upd_slidepot(m, erase, draw)
Flicmenu *m;
WORD erase, draw;
{
WORD new;
struct slidepot *s;

s = (struct slidepot *)m->text;
new = slide_where(m);
if (new != s->value)
	{
	see_1slide(m, s->value, erase);
	see_1slide(m, new, draw);
	s->value = new;
	}
}
#endif /* SLUFFED */

#ifdef SLUFFED
update_slidepot(m)
Flicmenu *m;
{
upd_slidepot(m, sblack, swhite);
}
#endif /* SLUFFED */

#ifdef SLUFFED
update_number_slider(m)
Flicmenu *m;
{
WORD new;
struct slidepot *s;

s = (struct slidepot *)m->text;
new = slide_where(m);
if (new != s->value)
	{
	s->value = new;
	draw_sel(m);
	}
}
#endif SLUFFED



extern see_number_slider();

#ifdef SLUFFED
feel_slidepot(m)
Flicmenu *m;
{
feelslide(m, update_slidepot);
}
#endif /* SLUFFED */

#ifdef SLUFFED
feel_number_slider(m)
Flicmenu *m;
{
feelslide(m, update_number_slider);
}
#endif SLUFFED

#ifdef SLUFFED
feelslide(m, update)
Flicmenu *m;
Vector update;
{
(*update)(m);
for (;;)
	{
	(*update)(m);
	wait_input();
	if (!PDN)
		break;
	}
}
#endif SLUFFED


static WORD processed_key;

static char break_menu, menu_ok;

close_menu_bad()
{
break_menu = 1;
menu_ok = 0;
}

close_menu()
{
break_menu = 1;
}

extern wait_click();

static
interp_menu(m, source, s, drawit, hide_for_sels)
Flicmenu *m;
Vector source;
Flicmenu *s;	/* initial selection if any */
int drawit;
int hide_for_sels;
{
Pull *ocpull;
Flicmenu *ocmenu;
char obreak;
int ok;

obreak = break_menu;
ocmenu = cur_menu;
ocpull = cur_pull;

menu_ok = 1;
break_menu = 0;
cur_menu = m;
cur_pull = NULL;
if (drawit)
	if (!draw_mp())
		{
		menu_ok = 0;
		goto OUT;
		}
if (s != NULL && s->feelme != NULL)
	(*s->feelme)(s);
while (!break_menu)
	{
	(*source)();
	if (PJSTDN || RJSTDN)
		{
		if (hide_for_sels)
			hide_mp();
		ok = rsel(m);
		if (hide_for_sels)
			draw_mp();
		if (!ok)
			{
			if (RJSTDN)
				{
				menu_ok  = 0;
				break;
				}
			}
		}
	else
		{
		if (hide_for_sels)
			hide_mp();
		menu_keys(m);
		if (hide_for_sels)
			draw_mp();
		}
	}
OUT:
if (drawit)
	hide_mp();
cur_pull = ocpull;
cur_menu = ocmenu;
break_menu = obreak;
return(menu_ok);
}

do_pmenu(m, s)
Flicmenu *m, *s;
{
return(interp_menu(m, wait_click, s, 1, 0));
}

hfs_do_menu(m)
Flicmenu *m;
{
return(interp_menu(m, wait_click, NULL, 1, 1));
}

do_menu(m)
Flicmenu *m;
{
return(interp_menu(m, wait_click, NULL, 1, 0));
}

nod_do_menu(m)
Flicmenu *m;
{
return(interp_menu(m, wait_click, NULL, 0, 0));
}

upc_char(c)
UBYTE c;
{
if (c >= 'A' && c <= 'Z')
	c += 'a' - 'A';
return(c);
}

menu_keys(m)
Flicmenu *m;
{
unsigned char c;

if (!m)
	return(0);
if (!key_hit)
	return(0);
c = key_in;
if (c == ' ')
	{
	break_menu = 1;
	return(0);
	}
c = upc_char(c);
if (c == 0)
	processed_key = key_in;
else
	processed_key = c;
return(rksel(m));
}



sub_menu_loop(selit, penfunc)
Vector selit;
Vector penfunc;
{
char obreak;
int ok;

obreak = break_menu;
break_menu = 0;
while (!break_menu)
	{
	wait_input();
	if (key_hit)
		{
		ok = pull_keys(&root_pull);
		if (ok < 0)	/* good 1st pull-equiv key, but then blew it */
			continue;
		else if (ok > 0)	/* got valid pull-equiv key selection */
			{
			(*selit)(menu_ix, sel_ix);
			continue;
			}
/* else drop through to let someomne else process keystroke */
		else if (!menu_keys(cur_menu))
			{
			if (a_break_key())
				break;
			}
		}
	else if (in_control_space())
		{
		if (in_pblock(0,0,cur_pull) )
			{
			if (interp_pull())
				{
				(*selit)(menu_ix, sel_ix);
				}
			}
		else if (PJSTDN||RJSTDN)
			{
			rsel(cur_menu);
			}
		}
	else if (RJSTDN)
		break;
	else if (PJSTDN && penfunc != NULL)
		(*penfunc)();
	}
break_menu = obreak;
}



static
rksel( m)
register Flicmenu *m;
{
if (m->next)
	{
	if ( rksel( m->next) )
		return(1);
	}
if (m->children)
	{
	if (rksel( m->children) )
		return(1);
	}
if (processed_key == m->key_equiv)
	{
	if (m->feelme)
		{
		(*m->feelme)( m);
		}
	return(1);
	}
return(0);
}

in_menu(m)
Flicmenu *m;
{
return ( uzx >= m->x && uzx <= m->x + m->width &&
	uzy >= m->y && uzy <= m->y + m->height);
}

rsel( m)
register Flicmenu *m;
{
if (m->next)
	{
	if ( rsel( m->next) )
		return(1);
	}
if (m->children)
	{
	if (rsel( m->children) )
		return(1);
	}
if (in_menu(m) )
	{
	if (PJSTDN)
		{
		if (!m->disabled && m->feelme)
			{
			(*m->feelme)( m);
			}
		return(1);
		}
	else if (RJSTDN)
		{
		if (m->optme)
			{
			(*m->optme)(m);
			}
		return(1);
		}
	}
return(0);
}

toggle_group(m)
Flicmenu *m;
{
*m->group = !*m->group;
draw_sel(m);
}

change_mode(m)
Flicmenu *m;
{
unhi_group(cur_menu, m->group);
*(m->group) = m->identity;
hi_group(cur_menu, m->group);
}

static char *cm_lines[] =
	{
	feelmenu_100 /* "Right click on the drawing screen" */,
	NULL,
	NULL,
	};

go_in_circles_message(s)
char *s;	/* what menu */
{
char buf[100];

sprintf(buf, feelmenu_101 /* "to return to %s panel." */, s);
cm_lines[1] = buf;
continu_box(cm_lines);
}

#ifdef SLUFFED
redraw_sel(m)
Flicmenu *m;
{
black_block(m);
draw_sel(m);
}
#endif SLUFFED

#ifdef SLUFFED
toggle_draw_mode(m)
Flicmenu *m;
{
if (vs.draw_mode == 0)
	vs.draw_mode = vs.wierd_draw_mode;
else
	vs.draw_mode = 0;
draw_sel(m);
}
#endif SLUFFED

tog_pen()
{
if (vs.pen_width == 0)
	vs.pen_width = vs.large_pen;
else
	vs.pen_width = 0;
}


toggle_pen(m)
Flicmenu *m;
{
tog_pen();
white_block(m);
draw_sel(m);
}

#ifdef SLUFFED
static Flicmenu *asl;
#endif SLUFFED

#ifdef SLUFFED
inc_sl()
{
struct slidepot *sl;

sl = (struct slidepot *)asl->text;
if (sl->value < sl->max)
	{
	sl->value++;
	draw_sel(asl);
	}
}
#endif SLUFFED

#ifdef SLUFFED
a_inc_slider(m,vector)
Flicmenu *m;
Vector vector;
{
hilight(m);
asl = (Flicmenu *)m->text;
repeat_on_pdn(vector);
draw_sel(m);
}
#endif SLUFFED

#ifdef SLUFFED
inc_slider(m)
Flicmenu *m;
{
a_inc_slider(m, inc_sl);
}
#endif SLUFFED

#ifdef SLUFFED
dec_sl()
{
struct slidepot *sl;

sl = (struct slidepot *)asl->text;
if (sl->value > sl->min)
	{
	--sl->value;
	draw_sel(asl);
	}
}
#endif SLUFFED

#ifdef SLUFFED
a_dec_slider(m,vector)
Flicmenu *m;
Vector vector;
{
hilight(m);
asl = (Flicmenu *)m->text;
repeat_on_pdn(vector);
draw_sel(m);
}
#endif SLUFFED

#ifdef SLUFFED
dec_slider(m)
Flicmenu *m;
{
a_dec_slider(m, dec_sl);
}
#endif SLUFFED

#ifdef SLUFFED
zero_group(m)
Flicmenu *m;
{
*(m->group) = 0;
}
#endif SLUFFED
