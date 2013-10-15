
/* Slider.c - routines for the famous 'numbered knob' slider */

#include "jimk.h"
#include "flicmenu.h"

char sl_overfl, sl_underfl;
extern struct cursor cleft, cright;
extern WORD mouse_connected;

#define ARR_WIDTH 11

static Flicmenu knob;
static Flicmenu *slm;
static char nbuf[10];
static struct qslider *qs;
static WORD qslastval;

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

qslide_where(m)
struct flicmenu *m;
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


see_qsleft(color)
int color;
{
register Flicmenu *m = slm;

knob.x = m->x;
knob.y = m->y;
knob.width = ARR_WIDTH;
knob.height = m->height;
a_frame(sgrey,&knob);
menu_cursor(&knob, color, &cleft);
}

see_qsright(color)
int color;
{
register Flicmenu *m = slm;

knob.x = m->x+m->width-ARR_WIDTH;
knob.y = m->y;
knob.width = ARR_WIDTH;
knob.height = m->height;
a_frame(sgrey,&knob);
menu_cursor(&knob, color, &cright);
}

/* see_qslider - the main slider SEEME */
see_qslider(m)
register Flicmenu *m;
{
slm = m;
gary_menu_back(m);
see_qsleft(sblack);
see_qsright(sblack);
resee_knob();
}

resee_knob()
{
make_knob(slm);
black_block(&knob);
menu_text(&knob,swhite);
}

erase_knob()
{
make_knob(slm);
white_block(&knob);		/* erase old position */
}

inc_qsl()
{
erase_knob();
if (*qs->value < qs->max)
	*qs->value += 1;
else /* ldg */
	sl_overfl=1;  /* slider overflow */
if (qs->update != NULL)
	(*qs->update)();
resee_knob();
}

dec_qsl()
{
erase_knob();
white_block(&knob);		/* erase old position */
if (*qs->value > qs->min)
	*qs->value -= 1;
else /* ldg */
	sl_underfl=1;  /* slider underflow */
if (qs->update != NULL)
	(*qs->update)();
resee_knob();
}


/* feel_qslider() - the main slider FEELME */
feel_qslider(m)
Flicmenu *m;
{
int dx;
int val;

if (!mouse_connected)
	{

	return;
	}
slm = m;
qs = m->text;
qslastval = *qs->value;
dx = uzx - m->x;
if (dx < ARR_WIDTH)		/* left arrow */
	{
	see_qsleft(sred);
	repeat_on_pdn(dec_qsl);
	see_qsleft(sblack);
	}
else if (dx >=  m->width - ARR_WIDTH)	/* right arrow */
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

