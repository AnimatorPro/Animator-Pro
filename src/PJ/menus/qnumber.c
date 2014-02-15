
/* qnumber.c - This module is for popping up little requestor to 
	get a single number or a single numeric string */

#include <stdlib.h>
#include "errcodes.h"
#include "memory.h"
#include "ptrmacro.h"
#include "wordwrap.h"
#include "menus.h"

typedef struct qnumwork {
	Menuhdr mh;
	char qstring[7]; /* (-32000) */
	Stringq sq;
	SHORT val;
	Qslider qs;
	Button hailb;
	Button qslb; /* qslider */
	Button stqb; /* note this and the next two are accessed as an array */
	Button okb;
	Button canb;
	Errcode (*update)(void *uddat, SHORT val);
	void *uddat;
} Qnumwork;

static void set_qnumstring(Qnumwork *qw, int drawit);

void see_hailing(Button *b)
{
	wwtext(b->root, b->root->font, 
		b->datme, b->x, b->y, b->width, b->height, 0, JUST_LEFT, 
		mc_grey(b), TM_MASK1, 0);
}
static void string_close(Button *b)
{
Qnumwork *qw = (Qnumwork *)get_button_hdr(b);
LONG lval;
int hit_enter;

	hit_enter = (STQ_ENTER & feel_string_req(b));
	lval = atol(qw->qstring);
	if(lval != (SHORT)lval) /* overflow !! */
	{
		hit_enter = 0; /* nope */
		set_qnumstring(qw,1); /* reset to old value */
	}
	else
		qw->val = lval;

	if(hit_enter)
		mb_close_ok(b);
	draw_buttontop(&(qw->qslb));
}
static void set_qnumstring(Qnumwork *qw, int drawit)
{
	setf_stringq(&(qw->stqb),drawit,"%d",qw->val);
}
static void update_qnumstring(void *qnumwork, Button *b)
{
	Qnumwork *qw = qnumwork;
	(void)b;

	set_qnumstring(qw,1);
	if(qw->update != NULL)
	{
		if((*qw->update)(qw->uddat,qw->val) < Success)
		{
			mb_close_cancel(&qw->canb);
			wait_penup();
		}
	}
}
void cleanup_qnumreq(Menuhdr *mh)
{
	pj_free(mh);
}


Errcode build_qnumreq(Wscreen *s, 
					  Menuhdr **pmh,
					  char *hailing,     /* text at top of box */ 
					  char **ok_cancel,  /* text for buttons */
					  struct image **arrows,    /* for slider */
					  SHORT initial, SHORT min, SHORT max,
					  Errcode (*update)(void *uddat, SHORT val),
					  void *uddat ) /* num vals */


/* build a menu for a quick number requestor the first button in the 
 * menuhdr->mbs list will be the string request field for do_reqloop 
 * initialization */
{
Vfont *f;
Qnumwork *qw;
Button *b;
SHORT cheight;  /* character height */
SHORT lheight;	/* line height (char + interline space) */
SHORT spwidth;	/* width of space */
SHORT bwidth;   /* button width */
SHORT bheight;  /* button height */
SHORT temp;
SHORT hborder;  /* horiz border */
SHORT twidth;	/* non border width */
int i;

	if(NULL == (*pmh = pj_zalloc(sizeof(Qnumwork))))
		return(Err_no_memory);

	qw = (Qnumwork *)(*pmh);
	qw->update = update;
	qw->uddat = uddat;

	/* calculate font based sizes */
	f = s->mufont;
	cheight = tallest_char(f);
	lheight = font_cel_height(f);
	spwidth = fchar_spacing(f," ");
	bwidth = fchar_spacing(f,"9") * 7; /* minimum size */
	if(bwidth < (temp = widest_line(f,ok_cancel,2) + spwidth*2))
		bwidth = temp;
	bheight = (9*lheight)/5; /* 9/5 */
	hborder = (4*spwidth)/3;
	twidth = bwidth*3 + spwidth*4;

	/* first button is for displaying hailing text */

	qw->hailb.x = hborder;
	qw->hailb.y = cheight;
	qw->hailb.width = twidth;
	qw->hailb.height = wwcount_lines(f,hailing,twidth,NULL)*lheight;
	qw->hailb.datme = hailing;
	qw->hailb.seeme = see_hailing;

	/* this one is the qslider */

	qw->val = initial;

	qw->qs.min = min;
	qw->qs.max = max;
	qw->qs.value = &qw->val;
	qw->qs.arrwidth = 12;
	qw->qs.arrows = arrows;
	qw->qs.update = update_qnumstring;
	qw->qs.data = qw;
	qw->qs.flags = QSL_LARRBOX|QSL_RARRBOX; 

	qw->qslb.x = hborder;
	qw->qslb.y = qw->hailb.y + qw->hailb.height + lheight;
	qw->qslb.width = twidth;
	qw->qslb.height = lheight + 3;
	qw->qslb.seeme = see_qslider,
	qw->qslb.feelme = feel_qslider,
	qw->qslb.datme = &(qw->qs);

	/* the next one is the stringq followed by okb and canb this will be
	 * the head of the buttons list */

	qw->sq.pxoff = spwidth/3;
	qw->sq.pyoff = font_ycent_oset(f,bheight);
	qw->sq.string = &(qw->qstring);
	qw->sq.dcount = 6;
	qw->sq.bcount = sizeof(qw->qstring)-1;

	b = &(qw->stqb);
	b->x = hborder;
	b->y = qw->qslb.y + qw->qslb.height + cheight;
	b->seeme = see_string_req;
	b->datme = &(qw->sq);
	b->key_equiv = '\t';

	i = 0;
	for(;;)
	{
	static VFUNC qnum_feelers[3] = {
		string_close,  	   /* stringq feel */
		mb_close_ok,       /* ok feel */
		mb_close_cancel,   /* cancel feel */ 
		};

		b->feelme = qnum_feelers[i];
		b->width = bwidth;
		b->height = bheight;
		if(i++ >= 2)
			break;
		++b;
		b->y = b[-1].y;
		b->x = b[-1].x + bwidth + spwidth*2;
		b->datme = *ok_cancel++;
		b->seeme = dcorner_text;
	}

	/* link 'em up and set flags */
	qw->mh.mbs = &(qw->stqb); /* note string field is first one */
	qw->stqb.next = &(qw->okb);
	qw->stqb.flags = MB_NORESCALE;
	qw->okb.next = &(qw->canb);
	qw->okb.flags = MB_NORESCALE;
	qw->canb.next = &(qw->qslb);
	qw->canb.flags = MB_NORESCALE;
	qw->qslb.flags = MB_NORESCALE;
	qw->qslb.next = &(qw->hailb);
	qw->hailb.flags = MB_NORESCALE;

	qw->mh.ioflags = (MBPEN|MBRIGHT|KEYHIT); /* any of this */
	qw->mh.flags = MENU_NORESCALE;
	qw->mh.seebg = seebg_white;
	qw->mh.width = twidth + hborder*2;
	qw->mh.height = b->y + bheight + lheight;
	menu_to_reqpos(s,&(qw->mh));
	set_qnumstring(qw,0);
	return(0);
}
SHORT get_qnumval(Menuhdr *mh)
{
	return(((Qnumwork *)mh)->val);
}

