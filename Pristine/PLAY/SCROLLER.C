
/*  scroller.c 
	Seeme and feelme routines to handle scrolling list of names. */

#include "jimk.h"
#include "flicmenu.h"

#define CHEIGHT (CH_HEIGHT)

extern WORD processed_key; /* ldg */

/* mostly cause of these two globals don't try to have more than 1
   scroller active at once... Also a menu with a scroll-bar calling another
   menu with a scroll bar may break.  Could fix this by making the
   iscroller routine part of the draw-scroller routine.... */

struct name_scroller *scroll;
Vector sredraw;

calc_scroll_pos(scroll, scroll_sel)
register Name_scroller *scroll;
register Flicmenu *scroll_sel;
{
WORD temp;
WORD spare_height;
WORD tname;

scroll->yoff = 1;
if ((temp = scroll->name_count - scroll->ycount) > 0)
	{
	tname = scroll->top_name;
	spare_height = scroll_sel->height - 2 - scroll->knob_height;
	if (tname > temp)	/* past end ?? */
		scroll->yoff = spare_height+1;
	else
		scroll->yoff += (spare_height*tname + (temp>>1))/temp ;
	}
}

iscroller(scr, names,scroll_sel,list_sel,ycount,redraw)
register Name_scroller *scr;
Name_list *names;
Flicmenu *scroll_sel;
register Flicmenu *list_sel;
int ycount;
Vector redraw;
{
WORD temp;

sredraw = redraw;
scroll = scr;
scr->name_list = names;
scr->name_count = els_in_list(names);
scr->ycount = ycount;
temp = scr->name_count - scr->ycount;
if (temp < 0)
	temp = 0;
if (scr->top_name > temp)
	scr->top_name = temp;
if (scr->name_count == 0)
	scr->knob_height = 0;
else if (scr->name_count < scr->ycount)
	scr->knob_height = scroll_sel->height - 2;
else
	{
	scr->knob_height = 
		(scroll_sel->height - 2) * scr->ycount/scr->name_count;
	if (scr->knob_height < 8)
		scr->knob_height = 8;
	}
scr->list_sel = list_sel;
scr->scroll_sel = scroll_sel;
calc_scroll_pos(scroll, scroll_sel);
}

/* figure out how many lines of text can fit in a scroller window */
scroll_ycount(list_sel)
Flicmenu *list_sel;
{
return((list_sel->height-2)/CHEIGHT);
}

redraw_scroller(scroll_sel, list_sel)
register Flicmenu *scroll_sel, *list_sel;
{
register Name_scroller *scroller;

scroll = scroller = (Name_scroller *)scroll_sel->text;

if (scroller->top_name > scroller->name_count - scroller->ycount)
	scroller->top_name = scroller->name_count - scroller->ycount;
if (scroller->top_name < 0)
	scroller->top_name = 0;
prt_list(list_sel);
calc_scroll_pos(scroller, scroll_sel);
draw_sel(scroll_sel);
}


end_clip_scroll()
{
int last;

last = scroll->name_count - scroll->ycount;
if (scroll->top_name > last)
	{
	scroll->top_name = last;
	}
}


fpagedown()
{
int nt, last;

scroll->top_name += scroll->ycount;
end_clip_scroll();
(*sredraw)();
}

fpageup()
{
int nt;

nt = scroll->top_name - scroll->ycount;
if (nt < 0)
	nt = 0;
scroll->top_name = nt;
(*sredraw)();
}

ffeelscroll(m)
Flicmenu *m;
{
fflscr(m, 1);
}

fflscr(m, realtime)
register Flicmenu *m;
int realtime;
{
register Name_scroller *scroller;
WORD temp;
WORD spare_height;
WORD last_top, first_top;

scroller = m->text;
if (scroller->name_count <= 0)
	return;
if ((temp = scroller->name_count - scroller->ycount) <= 0)
	return;
if (processed_key==PAGEUP) /* ldg */
	{
	fpageup();
	return;
	}
else if (processed_key==PAGEDN)
	{
	fpagedown();
	return;
	}
if (uzy > m->y + scroller->yoff + scroller->knob_height)
	repeat_on_pdn(fpagedown);
else if (uzy < m->y + scroller->yoff)
	repeat_on_pdn(fpageup);
else
	{
	spare_height = m->height - 2 - scroller->knob_height;
	firstx = uzx;
	firsty = uzy;
	first_top = last_top = scroller->top_name;
	for (;;)
		{
		if (!PDN)
			break;
		if (mouse_moved)
			{
			scroller->top_name = first_top +
				((uzy-firsty)*scroller->name_count+(spare_height>>1)) /
				spare_height;
			if (scroller->top_name < 0)
				scroller->top_name = 0;
			if (scroller->top_name > temp)
				scroller->top_name = temp;
			if (last_top != scroller->top_name)
				{
				last_top = scroller->top_name;
				if (realtime)
					(*sredraw)();
				else
					{
					calc_scroll_pos(scroller, m);
					draw_sel(m);
					}
				}
			}
		wait_input();
		}
	if (!realtime)
		(*sredraw)();
	}
}

#define TOPNAME_OFF 2
print_list(m)
Flicmenu *m;
{
a_frame(sgrey, m);
prt_list(m);
}

prt_list(m)
register Flicmenu *m;
{
register Name_list *n;
register Name_scroller *scroller;
WORD name_count, line_count;
WORD cheight, cwidth;
WORD twidth;
WORD xoff, yoff;
WORD i;
char *string;

colrop(swhite, m->x+1, m->y+1, 
	m->width-2, m->height-2);
scroller = (Name_scroller *)m->text;
n = scroller->name_list;
twidth = cwidth = m->width/CH_WIDTH;
cheight = scroller->ycount;
twidth *= CH_WIDTH;
xoff = m->x + ((m->width-twidth)>>1) + 1;
yoff = m->y + TOPNAME_OFF;
i = scroller->top_name;
while (--i >= 0)
	{
	if (n == NULL)
		return;
	n = n->next;
	}

i = cheight;
if (scroller->name_count < i)
	i = scroller->name_count;
while (--i >= 0)
	{
	if (n == NULL)
		break;
	string = n->name;
	twidth = strlen(string);
	if (twidth > cwidth)
		twidth = cwidth;
	gtext(   string, xoff, yoff, sblack);
	n = n->next;
	yoff += CHEIGHT;
	}
}

see_scroll(m)
Flicmenu *m;
{
register Name_scroller *scroller;

a_block(sgrey, m);
scroller = (Name_scroller *)m->text;
if (scroller->name_count > 0)
	{
	colrop(sblack, m->x+1, m->y+scroller->yoff, m->width-2,
		scroller->knob_height);
	}
}

void *
which_sel(m)
Flicmenu *m;
{
WORD y, py;
register Name_scroller *scroll;
register Name_list *thread;

scroll = (Name_scroller *)m->text;
y = uzy - m->y - TOPNAME_OFF;
y /= CHEIGHT;
py = y*CHEIGHT + m->y + TOPNAME_OFF;
y += scroll->top_name;
if (y < scroll->name_count)
	{
	xorrop(sblack^swhite, m->x+1, py, m->width-2, CHEIGHT-1);
	wait_penup();
	xorrop(sblack^swhite, m->x+1, py, m->width-2, CHEIGHT-1);
	thread = scroll->name_list;
	while (--y >= 0)
		thread = thread->next;
	return(thread);
	}
return(NULL);
}

char *
sel_name(m)
Flicmenu *m;
{
Name_list *n;

if ((n = which_sel(m))!=NULL)
	return(n->name);
else
	return(NULL);
}



fincu(m)
Flicmenu *m;
{
if (scroll->top_name > 0)
	{
	--scroll->top_name;
	(*sredraw)();
	}
}

fincup(m)
Flicmenu *m;
{
hilight(m);
repeat_on_pdn(fincu);
draw_sel(m);
}

fincd()
{
scroll->top_name++;
end_clip_scroll();
(*sredraw)();
}

fincdown(m)
Flicmenu *m;
{
hilight(m);
repeat_on_pdn(fincd);
draw_sel(m);
}
