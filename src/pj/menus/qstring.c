
/* qstring.c - This module is for popping up little requestor to 
	get a single string */

#include "errcodes.h"
#include "memory.h"
#include "ptrmacro.h"
#include "wordwrap.h"
#include "menus.h"

typedef struct qstrwork {
	Menuhdr mh;
	Stringq sq;
	Button hailb;
	Button stqb;
	Button okb;
	Button canb;
} Qstrwork;

static void string_close(Button *b)
{
	if(feel_string_req(b) & STQ_ENTER)
		mb_close_ok(b);
}
void cleanup_qstrreq(Menuhdr *mh)
{
	pj_free(mh);
}

Errcode build_qstrreq(Wscreen *s, 
					  Menuhdr **pmh,
					  char *hailing,     /* text at top of box */ 
					  char **ok_cancel,  /* text for buttons */
					  char *strbuf,
					  int strlength)
/* build a menu for a quick string requestor the first button in the 
 * menuhdr->mbs list will be the string request field for do_reqloop 
 * initialization */
{
Vfont *f;
Qstrwork *qw;
Button *b;
SHORT cheight;  /* character height */
SHORT lheight;	/* line height (char + interline space) */
SHORT spwidth;	/* width of space */
SHORT bwidth;   /* button width */
SHORT bheight;  /* button height */
SHORT temp;
SHORT hborder;  /* horiz border */
SHORT twidth;	/* non border width */
SHORT strwid;	/* width of string button */
SHORT strdchars;
SHORT yoff;

	if(NULL == (*pmh = pj_zalloc(sizeof(Qstrwork))))
		return(Err_no_memory);

	qw = (Qstrwork *)(*pmh);

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

	strdchars = strlength;
	if (strdchars > 32)
		strdchars = 32;
	strwid = fchar_spacing(f, "M")*strdchars + 2*spwidth;

	twidth = bwidth*3 + spwidth*4;
	if (twidth < strwid)
		twidth = strwid;
	strwid -= 2*spwidth;

	/* first button is for displaying hailing text */
	qw->hailb.x = hborder;
	qw->hailb.y = cheight;
	qw->hailb.width = twidth;
	qw->hailb.height = wwcount_lines(f,hailing,twidth,NULL)*lheight;
	qw->hailb.datme = hailing;
	qw->hailb.seeme = see_hailing;
	qw->hailb.flags = MB_NORESCALE;

	twidth += 2*hborder;

	/* make up the stringq structure */
	qw->sq.pxoff = spwidth/3;
	qw->sq.pyoff = font_ycent_oset(f,bheight);
	qw->sq.string = strbuf;
	qw->sq.dcount = strdchars;
	qw->sq.bcount = strlength;
	init_stq_string(&qw->sq);

	/* make up the stringq button */
	b = &(qw->stqb);
	b->x = (twidth - strwid)>>1;
	yoff = b->y = qw->hailb.y + qw->hailb.height + cheight;
	b->width = strwid;
	b->height = bheight;
	b->seeme = see_string_req;
	b->datme = &(qw->sq);
	b->feelme = string_close;
	b->flags = MB_NORESCALE;
	b->key_equiv = '\t';

	/* The ok button */
	yoff += bheight + cheight;
	b = &(qw->okb);
	b->feelme = mb_close_ok;
	b->width = bwidth;
	b->height = bheight;
	b->x = hborder;
	b->y = yoff;
	b->datme = *ok_cancel++;
	b->seeme = dcorner_text;
	b->flags = MB_NORESCALE;

	/* The cancel button */
	b = &(qw->canb);
	b->feelme = mb_close_cancel;
	b->width = bwidth;
	b->height = bheight;
	b->x = twidth - bwidth - hborder;
	b->y = yoff;
	b->datme = *ok_cancel;
	b->seeme = dcorner_text;
	b->flags = MB_NORESCALE;

	/* link 'em up */
	qw->mh.mbs = &(qw->stqb); /* note string field is first one */
	qw->stqb.next = &(qw->okb);
	qw->okb.next = &(qw->canb);
	qw->canb.next = &(qw->hailb);

	qw->mh.ioflags = (MBPEN|MBRIGHT|KEYHIT); /* any of this */
	qw->mh.flags = MENU_NORESCALE;
	qw->mh.seebg = seebg_white;
	qw->mh.width = twidth;
	qw->mh.height = b->y + bheight + lheight;
	menu_to_reqpos(s,&(qw->mh));
	return(0);
}

