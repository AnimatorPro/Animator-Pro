/* slider.c - functions to implement our famous "number on the knob"
   slider buttons */

#include <stdio.h>
#include "imath.h"
#include "input.h"
#include "memory.h"
#include "menus.h"
#include "rastext.h"

#ifdef SLUFFED
int clip_to_slider(int val, Qslider *qs)
/* Return val clipped to slider */
{
if (val < qs->min)
	val = qs->min;
else if (val > qs->max)
	val = qs->max;
return(val);
}
#endif /* SLUFFED */

static void format_shortp1(void *val, char *buf, void *data)
{
	(void)data;
	sprintf(buf, "%d", *(short *)val + 1);
}
static void format_short(void *val, char *buf, void *data)
{
	(void)data;
	sprintf(buf, "%d", *(short *)val);
}

Qslfmt qsfmt_short = {
    format_short,	
	NULL,
	NULL,
	NULL,
};

Qslfmt qsfmt_shortp1 = {
    format_shortp1,	
	NULL,
	NULL,
	NULL,
};

typedef struct qslwork {
	Button *slb;
	Button knob;
	char nbuf[12];
	Qslider *qs;
	Vfont *font;
	SHORT arrwidth;
	Qslfmt fmt;
	Menuhdr *mhdr;
} Qslwork;

static void resee_knob(Qslwork *qw);

static void init_qslwork(Button *slb,Qslwork *qw)
{
Rscale *scale;
Qslfmt *fmt;

	qw->slb = slb;
	qw->mhdr = get_button_hdr(slb);
	clear_struct(&qw->knob);
	qw->knob.flags = slb->flags & MB_ROOTISWNDO;
	qw->knob.root = slb->root;
	qw->font = slb->root->font;
	qw->knob.datme = qw->nbuf;
	qw->knob.flags |= MB_NORESCALE;
	qw->qs = (struct qslider *)(slb->datme);
	scale = mb_get_menu_scale(slb);
	qw->arrwidth = rscale_x(scale,qw->qs->arrwidth);
	if((fmt = qw->qs->fmt) == NULL)
		qw->fmt = qsfmt_short;
	else
		qw->fmt = *fmt;
}

static void make_knob(register Qslwork *qw)
{
register struct qslider *slider;
SHORT nval;
SHORT nmax;
SHORT swidth;
SHORT nwidth;
SHORT width;
SHORT val;

	slider = qw->qs;
	(*(qw->fmt.format))(slider->value,qw->nbuf,slider->data);

	val = *((SHORT *)slider->value);
	if (val < slider->min)
		val = slider->min;
	if (val > slider->max)
		val = slider->max;

	swidth = qw->slb->width - (2 + 2*qw->arrwidth);
	nval = val - slider->min;
	nmax = slider->max - slider->min + 1;
	width = pj_uscale_by(swidth, nval+1, nmax) - pj_uscale_by(swidth, nval, nmax);
	nwidth = fstring_width(qw->font,qw->nbuf) + fchar_spacing(qw->font," ")/2;
	if (width < nwidth)
		width = nwidth;
	swidth -= width;
	qw->knob.width = width;
	qw->knob.height = qw->slb->height-4;
	qw->knob.x = qw->arrwidth+1+qw->slb->x;
	if(nmax > 1)
		qw->knob.x += pj_uscale_by(swidth, nval, nmax-1);

	qw->knob.y = qw->slb->y+2;
}

static int qslide_where(Qslwork *qw)
{
register struct qslider *s;
SHORT width;
SHORT x;

	s = qw->qs;
	x = icb.mx - qw->slb->x - 2 - qw->arrwidth;
	width = qw->slb->width - 4 - 2*qw->arrwidth;
	if (x < 0)
		return(s->min);
	if (x >= width)
		return(s->max);
	return(pj_uscale_by(x, s->max - s->min + 1, width) + s->min);
}

static void see_qsleft(Qslwork *qw,Pixel color)
{
Image *arrow;

	qw->knob.x = qw->slb->x;
	qw->knob.y = qw->slb->y;
	qw->knob.width = qw->arrwidth;
	qw->knob.height = qw->slb->height;
	if((NULL != (arrow = (Image *)(qw->qs->arrows)))
		&& (NULL != (arrow = ((Image **)arrow)[QSL_LARROW])))
	{
		mb_centimage(&qw->knob, color, arrow);
	}
	if(qw->arrwidth && (qw->qs->flags & QSL_LARRBOX))
		mc_frame(&qw->knob,MC_GREY);
}

static void see_qsright(Qslwork *qw,Pixel color)
{
Image *arrow;

	qw->knob.x = qw->slb->x+qw->slb->width-qw->arrwidth;
	qw->knob.y = qw->slb->y;
	qw->knob.width = qw->arrwidth;
	qw->knob.height = qw->slb->height;
	if((NULL != (arrow = (Image *)(qw->qs->arrows)))
		&& (NULL != (arrow = ((Image **)arrow)[QSL_RARROW])))
	{
		mb_centimage(&qw->knob, color, arrow);
	}
	if(qw->arrwidth && (qw->qs->flags & QSL_RARRBOX))
		mc_frame(&qw->knob,MC_GREY);
}

void see_qslider(register Button *b)
{
Wscreen *s = b->root->w.W_screen;
Qslwork qw;
SHORT x,width;

	init_qslwork(b,&qw);
	a_block(b,s->SWHITE);
	x = b->x;
	width = b->width;
	if(qw.arrwidth)
	{
		x += qw.arrwidth - 1;
		width -= ((qw.arrwidth-1)<<1);
	}
	draw_quad((Raster *)b->root, s->SGREY, x, b->y, width, b->height);
	see_qsleft(&qw,s->SBLACK);
	see_qsright(&qw,s->SBLACK);
	resee_knob(&qw);
}
static void resee_knob(Qslwork *qw)
{
	make_knob(qw);
	black_block(&qw->knob);
	mb_centext(&qw->knob,qw->knob.root->w.W_screen->mc_colors[MC_WHITE],
			   qw->knob.datme);
}

static void erase_knob(Qslwork *qw)
{
	make_knob(qw);
	white_block(&qw->knob);	/* erase old position */
}

static void delta_qsl(Qslwork *qw,SHORT delta)
{
register struct qslider *qs = qw->qs;
SHORT val;

	/* possible that update function closed or hid menu! */
	if(!MENU_ISUP(qw->mhdr))
		return;

	val = delta + *((SHORT *)qs->value);

	if(delta > 0)
	{
		if(val > qs->max)
		{
			if(qs->flags & QSL_MAXWRAP)
				val = qs->min;
			else
				return;
		}
	}
	else if(val < qs->min)
	{
		if(qs->flags & QSL_MINWRAP)
			val = qs->max;
		else
			return;
	}

	if(val == *((SHORT *)qs->value)) /* don't flash screen if nothing done */
		return;

	erase_knob(qw);
	*((SHORT *)qs->value) = val;
	if (qs->update != NULL)
	{
		(*qs->update)(qs->data,qw->slb);
		/* possible that update function closed or hid menu! */
		if(!MENU_ISUP(qw->mhdr))
			return;
	}
	resee_knob(qw);
}
static void inc_qsl(void *qw)
{
	delta_qsl((Qslwork *)qw, 1);
}
static void dec_qsl(void *qw)
{
	delta_qsl((Qslwork *)qw, -1);
}

Boolean in_left_arrow(Button *slb)
{
SHORT arrwidth;

	if(0 == (arrwidth = ((Qslider *)(slb->datme))->arrwidth))
		return(0);
	return(icb.mx - slb->x < mb_mscale_x(slb,arrwidth));
}

Boolean in_right_arrow(Button *slb)
{
SHORT arrwidth;

	if(0 == (arrwidth = ((Qslider *)(slb->datme))->arrwidth))
		return(0);
	return(icb.mx - slb->x >= slb->width - mb_mscale_x(slb,arrwidth));
}

void feel_qslider(Button *b)
{
Wscreen *s = b->root->w.W_screen;
int val;
Qslwork qw;

	init_qslwork(b,&qw);
	if(in_left_arrow(b))
	{
		see_qsleft(&qw,s->SRED);
		repeat_on_pdn(dec_qsl,&qw); 
		/* possible that update function closed or hid menu! */
		if(MENU_ISUP(qw.mhdr))
			see_qsleft(&qw,s->SBLACK);
	}
	else if(in_right_arrow(b))
	{
		see_qsright(&qw,s->SRED);
		repeat_on_pdn(inc_qsl,&qw);
		/* possible that update function closed or hid menu! */
		if(MENU_ISUP(qw.mhdr))
			see_qsright(&qw,s->SBLACK);
	}
	else					/* in the middle somewhere */
	{
		while(ISDOWN(MBPEN))
		{
			val = qslide_where(&qw);
			if (val != *(SHORT *)qw.qs->value)
			{
				erase_knob(&qw);
				*(SHORT *)qw.qs->value = val;
				if(qw.qs->update != NULL)
				{
					(*qw.qs->update)(qw.qs->data,b);
					/* possible that update function closed or hid menu! */
					if(!MENU_ISUP(qw.mhdr))
						break;
				}
				resee_knob(&qw);
			}
			wait_input(MMOVE|MBPEN|MBPUP);
		}
	}
}
