
/* Feelmenu.c - process mouse input over a panel menu surface.
   Figure out which button we're on traversing menu tree recursively.
   Also some routines to move all the buttons on a menu tree. */

#include "jimk.h"
#include "flicmenu.h"


Flicmenu *cur_menu;	/* pointer to current menu */

upc_char(c)
UBYTE c;
{
if (c >= 'A' && c <= 'Z')
	c += 'a' - 'A';
return(c);
}



/* Check to make sure we stay onscreen, then move whole menu tree */
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

/* Move a whole menu tree */
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


static 
pdn_timeout(timeout)
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

/* Deal with buttons that auto-repeat while the mouse is down.  Pass in
   function to repeat.  repeat_on_pdn() will take care of how often to
   call it. */
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



WORD processed_key;

char break_menu, menu_ok;

close_menu()
{
break_menu = 1;
}

/* Process keystroke a bit before going into recursive keyboard selection
   routine (rksel).  Break out of menu if get spacebar, otherwise 
   convert key_in to upper case and try to call the right button. */
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
if (c >= 'A' && c <= 'Z')
	c += 'a' - 'A';
if (c == 0)
	processed_key = key_in;
else
	processed_key = c;
return(rksel(m));
}


/* Main menu interpreter */
interp_menu(m, source, s)
struct flicmenu *m;	/* Root menu button */
Vector source;		/* What function we call to get input */
struct flicmenu *s;	/* Initial pre-selection if any */
{
Flicmenu *ocmenu;
char obreak;

obreak = break_menu;
ocmenu = cur_menu;

menu_ok = 1;
break_menu = 0;
cur_menu = m;
draw_mp();
if (s != NULL && s->feelme != NULL)
	(*s->feelme)(s);
while (!break_menu)
	{
	(*source)();
	if (PJSTDN || RJSTDN)
		{
		if (!rsel(m))
			{
			menu_ok  = 0;
			break;
			}
		}
	else
		menu_keys(m);
	}
hide_mp();
cur_menu = ocmenu;
break_menu = obreak;
return(menu_ok);
}


/* Recursively decide if any menu button wants this key-press */
rksel( m)
register struct flicmenu *m;
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

/* See if mouse is in a particular menu button */
in_menu(m)
Flicmenu *m;
{
return ( uzx >= m->x && uzx <= m->x + m->width &&
	uzy >= m->y && uzy <= m->y + m->height);
}

/* Recursively decide which menu button to select and call it's FEELME */
rsel( m)
register struct flicmenu *m;
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

/* toggle_group - the basic FEELME for toggle buttons */ 
toggle_group(m)
struct flicmenu *m;
{
*m->group = !*m->group;
draw_sel(m);
}

/* change_mode - the basic FEELME for radio buttons.
   Unhilight all other buttons in group, select and
   hilight this one. */
change_mode(m)
Flicmenu *m;
{
unhi_group(cur_menu, m->group);
*(m->group) = m->identity;
hi_group(cur_menu, m->group);
}
