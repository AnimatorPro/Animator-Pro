
/* slider.c - functions to implement our famous "number on the knob"
   slider gadgets */

#include "jimk.h"
#include "flicmenu.h"

extern struct cursor cleft, cright;
extern struct cursor cup, cdown, zin, zout;

static struct cursor *larrs[3] = {&cleft, &cup, &zout};
static struct cursor *rarrs[3] = {&cright, &cdown, &zin};


#define ARR_WIDTH 11

static Flicmenu knob;
static Flicmenu *slm;
static char nbuf[10];
static struct qslider *qs;
static WORD qslastval;

static
make_knob(ssel)
register Flicmenu *ssel;
{
register struct qslider *slider;
WORD nval;
WORD nmax;
WORD swidth;
WORD nwidth;
WORD width;
WORD val;

slider = (struct qslider *)ssel->text;
val = *(slider->value);
sprintf(nbuf, "%d", val+slider->voff);
if (val < slider->min)
	val = slider->min;
if (val > slider->max)
	val = slider->max;
swidth = ssel->width - 4 - 2*ARR_WIDTH;
nval = val - slider->min;
nmax = slider->max - slider->min + 1;
width = uscale_by(swidth, nval+1, nmax) - uscale_by(swidth, nval, nmax);
nwidth = strlen(nbuf)*CH_WIDTH;
if (width < nwidth)
	width = nwidth;
swidth -= width;
knob.width = width;
knob.height = ssel->height-4;
if (nmax <= 1)
	knob.x = ARR_WIDTH+2+ssel->x;
else
	knob.x = ARR_WIDTH+2+ssel->x + uscale_by(swidth, nval, nmax-1);
knob.y = ssel->y+2;
knob.text = nbuf;
}

static
qslide_where(m)
Flicmenu *m;
{
register struct qslider *s;
WORD width;
WORD x;

s = (struct qslider *)m->text;
x = uzx - m->x - 2 - ARR_WIDTH;
width = m->width - 4 - 2*ARR_WIDTH;
if (x < 0)
	return(s->min);
if (x >= width)
	return(s->max);
return(uscale_by(x, s->max - s->min + 1, width) + s->min);
}


static
see_qtleft(color)
{
register Flicmenu *m = slm;

knob.x = m->x;
knob.y = m->y;
knob.width = ARR_WIDTH;
knob.height = m->height;
menu_cursor(&knob, color, larrs[m->identity]);
}

static
see_qsleft(color)
int color;
{
see_qtleft(color);
a_frame(sgrey,&knob);
}

static
see_qtright(color)
int color;
{
register Flicmenu *m = slm;

knob.x = m->x+m->width-ARR_WIDTH;
knob.y = m->y;
knob.width = ARR_WIDTH;
knob.height = m->height;
menu_cursor(&knob, color, rarrs[m->identity]);
}

static
see_qsright(color)
{
see_qtright(color);
a_frame(sgrey,&knob);
}


see_qtslider(m)
register Flicmenu *m;
{
slm = m;
a_block(swhite,m);
draw_frame(sgrey,  m->x+ARR_WIDTH, 
	m->y,  m->x + m->width+1-ARR_WIDTH, m->y + m->height+1);
see_qtleft(sblack);
see_qtright(sblack);
resee_knob();
}

see_qslider(m)
register Flicmenu *m;
{
slm = m;
gary_menu_back(m);
see_qsleft(sblack);
see_qsright(sblack);
resee_knob();
}


static
resee_knob()
{
make_knob(slm);
black_block(&knob);
menu_text(&knob,swhite);
}

static
erase_knob()
{
make_knob(slm);
white_block(&knob);		/* erase old position */
}

static
inc_qsl()
{
erase_knob();
if (*qs->value < qs->max)
	*qs->value += 1;
if (qs->update != NULL)
	(*qs->update)();
resee_knob();
}

static
dec_qsl()
{
erase_knob();
white_block(&knob);		/* erase old position */
if (*qs->value > qs->min)
	*qs->value -= 1;
if (qs->update != NULL)
	(*qs->update)();
resee_knob();
}

in_left_arrow(m)
Flicmenu *m;
{
return(uzx - m->x < ARR_WIDTH);
}

in_right_arrow(m)
Flicmenu *m;
{
return(uzx - m->x >=  m->width - ARR_WIDTH);	/* right arrow */
}

feel_qslider(m)
Flicmenu *m;
{
int val;

slm = m;
qs = m->text;
qslastval = *qs->value;
if (in_left_arrow(m))
	{
	see_qsleft(sred);
	repeat_on_pdn(dec_qsl);
	see_qsleft(sblack);
	}
else if (in_right_arrow(m))
	{
	see_qsright(sred);
	repeat_on_pdn(inc_qsl);
	see_qsright(sblack);
	}
else					/* in the middle somewhere */
	{
	while (PDN)
		{
		val = qslide_where(m);
		if (val != *qs->value)
			{
			erase_knob();
			*qs->value = val;
			if (qs->update != NULL)
				(*qs->update)();
			resee_knob();
			}
		wait_input();
		}
	}
}

