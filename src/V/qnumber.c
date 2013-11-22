
/* qnumber.c - This module is for popping up little requestor to 
	get a single number or a single string */

#include "jimk.h"
#include "flicmenu.h"
#include "gemfont.h"
#include "commonst.h"

extern gary_menu_back(), menu_text_box(), see_string_req(), 
	close_menu(), close_menu_bad(), dcorner_text(),
	string_close(), feel_string_req(), see_qslider(), qn_feel_qslider();

static char qstring[11];
static struct stringq qstring_stringq =
	{
	2, 2, 7, 10, 0, qstring, NULL, 0, 0,
	};

static int qnum;

static struct qslider qnu_slider = 
	{
	0, 100, &qnum, 0, NULL,
	};

/*** Button Data ***/
static Flicmenu qnu_can_sel = {
	NONEXT,
	NOCHILD,
	190, 112, 42, 12,
	cst_cancel,
	dcorner_text,
	close_menu_bad,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu qnu_ok_sel = {
	&qnu_can_sel,
	NOCHILD,
	143, 112, 34, 12,
	cst_ok,
	dcorner_text,
	close_menu,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu qnu_sli_sel = {
	&qnu_ok_sel,
	NOCHILD,
	87, 96, 145, 10,
	&qnu_slider,
	see_qslider,
	qn_feel_qslider,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu reqstring_sel =
	{
	&qnu_sli_sel,
	NOCHILD,
	87, 112, 7*6, 12,
	&qstring_stringq,	
	see_string_req,
	string_close,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

static Flicmenu reqhail_sel =
	{
	&reqstring_sel,
	NOCHILD,
	84, 73, 153, 19,
	NOTEXT,	/* filled in by caller */
	menu_text_box,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

static Flicmenu qreq_menu =
	{
	NONEXT,
	&reqhail_sel,
	80, 70, 160, 60,
	NOTEXT,
	gary_menu_back,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

static
menu_text_box(m)
Flicmenu *m;
{
to_upper(m->text);
wwtext(&vf, &sixhi_font,
	m->text, m->x+2, m->y+2, m->width-3, m->height-3, sgrey,
	a1blit, 0, 0);
}

static
string_close(m)
Flicmenu *m;
{
if (feel_string_req(m))
	close_menu();
qnum = atoi(qstring);
draw_sel(&qnu_sli_sel);
}

static
qn_feel_qslider(m)
Flicmenu *m;
{
feel_qslider(m);
init_qnums();
draw_sel(&reqstring_sel);
}

static
init_qnums()
{
sprintf(qstring, "%d", qnum);
init_stq_string(&qstring_stringq);
}

qreq_number(hailing, inum, min, max)
char *hailing;
int *inum,  min, max;
{
int ok;

reqhail_sel.text = hailing;
qnu_slider.min = min;
qnu_slider.max = max;

qnum = *inum;
init_qnums();

clip_rmove_menu(&qreq_menu, uzx - (qreq_menu.x + qreq_menu.width/2),
	uzy - (qreq_menu.y + qreq_menu.height/2) );
if ((ok = do_pmenu(&qreq_menu, &reqstring_sel)) != 0)
	*inum = qnum;
return(ok);
}


