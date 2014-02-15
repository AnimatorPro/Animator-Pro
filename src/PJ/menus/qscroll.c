#include "errcodes.h"
#include "imath.h"
#include "input.h"
#include "linklist.h"
#include "memory.h"
#include "menus.h"
#include "ptrmacro.h"
#include "rastext.h"
#include "scroller.h"

typedef struct qscrollwork {
	Menuhdr mh;
	Name_scroller scroll;
	Stringq stq;
	Button buts[8];
} Qscrollwork;

static Qscrollwork *gqsw;	

extern Image ctriup, ctridown;


/* Defines so can pretend buttons in array have names */
#define cursel buts[0]
#define hailb buts[1]
#define uparr buts[2]
#define downarr buts[3]
#define sbar buts[4]
#define slist buts[5]
#define okb buts[6]
#define canb buts[7]

/* Constants in lo res pixels */
#define LO_ARROW_W 13	/* Arrow dimensions */
#define LO_ARROW_H 10
#define IWD 4		/* space between list box and ok/cancel... */
#define IBD 2		/* Space between buttons and outside of window */

static void accept_name(Button *b)
{
	if(feel_string_req(b) & STQ_ENTER)
		mb_gclose_ok(b);
}
static void feel_1_scroll(Button *list_sel,void *rast,int x,int y,
						  Names *entry, int why)
{
	(void)rast;
	(void)x;
	(void)y;

	setf_stringq(&gqsw->cursel, 1, "%s", entry->name);
	if(why & (SCR_MDHIT|SCR_ENTER))
		mb_gclose_ok(list_sel);
}


#define MINLINES 5		/* smallest # of lines in scroll list area */

Errcode build_qscroller(
	char *result,		/* string we fill in, and start out with */
	Wscreen *s,			/* Screen to put it on */
	Menuhdr **pmh,		/* Menu header to store result of build */
	char *hailing,  	/* Character string for move/title area */
	Names *items, 		/* List of things to put in scroller */
	SHORT lines,		/* # of lines visible in scroller.  At least 5! */
	char *ok,			/* String for ok button */
	char *cancel,		/* String for cancel button */
	SHORT *ipos)		/* Initial scroller position */
{
SHORT font_height;
Vfont *font;
SHORT maxchars;
Qscrollwork *qsw;
SHORT rightwid;		/* width of right side */
SHORT rightoff;
SHORT okcanmax;		/* max width of ok/cancel strings in pixels */
unsigned int i;

/* some error checking */
if (items == NULL)
	return(Err_not_found);
if ((slist_el((Slnode *)items, *ipos)) == NULL)
	*ipos = 0;
if (lines < MINLINES)
	lines = MINLINES;

/* Get workspace structure plus enough to hold longest single string */
if ((gqsw = qsw = pj_zalloc(sizeof(*qsw))) == NULL)
	return(Err_no_memory);

/* initialize string requestor */
maxchars = longest_name(items);
qsw->stq.string = result;
qsw->stq.bcount = qsw->stq.dcount = maxchars;
init_stq_string(&qsw->stq);

/* calculate dimensions of list button (mostly) */
font = s->mufont;
font_height = font_cel_height(font);
qsw->slist.width = qsw->cursel.width = 
	widest_name(font, items) + 2*scroll_name_xoff(font);
qsw->slist.height = scroll_names_ysize(font, lines);

/* calculate arrow dimensions (mostly) */
qsw->uparr.width = qsw->downarr.width = qsw->sbar.width = 
	rscale_x(&s->menu_scale, LO_ARROW_W);
qsw->uparr.height = qsw->downarr.height = rscale_x(&s->menu_scale, LO_ARROW_H);

/* calculate dimensions of scroll-bar */
qsw->sbar.height = qsw->slist.height - (qsw->uparr.height<<1) + 2;
qsw->sbar.width = qsw->uparr.width-2;

/* figure out width of right side of menu (big enough for ok, cancel or
   widest string from scroller */
okcanmax = intmax((int)fstring_width(font, ok), 
	(int)fstring_width(font, cancel)) + 4;
rightwid = intmax(qsw->slist.width, okcanmax);
if (okcanmax < (rightwid>>1))
	okcanmax = rightwid>>1;

/* calculate dimensions of entire dialog */
qsw->hailb.width = 
	3*IBD + IWD + qsw->uparr.width + qsw->slist.width + rightwid + 2;

if((qsw->mh.width = fstring_width(font,hailing)+2) > qsw->hailb.width)
	qsw->hailb.width = qsw->mh.width;
else
	qsw->mh.width = qsw->hailb.width;

qsw->mh.height = qsw->slist.height + font_height + IBD + 2;


/* finish up hailing/move menu area */
qsw->hailb.height = font_height;
qsw->hailb.x = qsw->hailb.y = 0;
qsw->hailb.seeme = see_titlebar;
qsw->hailb.datme = hailing;
qsw->hailb.feelme = mb_clipmove_menu;
qsw->hailb.optme = mb_menu_to_bottom;

/* finish up arrow buttons */
qsw->uparr.x = qsw->downarr.x = IBD + 1;
qsw->downarr.y = qsw->uparr.y = font_height + IBD;
qsw->downarr.y += qsw->sbar.height + qsw->downarr.height - 2;
qsw->uparr.seeme = qsw->downarr.seeme = ccorner_image;
qsw->uparr.datme = &ctriup;
qsw->downarr.datme = &ctridown;
qsw->uparr.feelme = scroll_incup;
qsw->downarr.feelme = scroll_incdown;
qsw->uparr.group = qsw->downarr.group = (void *)(&qsw->scroll);
qsw->uparr.identity = qsw->downarr.identity = -1;
qsw->uparr.key_equiv = UARROW;
qsw->downarr.key_equiv = DARROW;

/* finish up scroll bar */
qsw->sbar.y = qsw->uparr.y + qsw->uparr.height - 1;
qsw->sbar.x = qsw->uparr.x + 1;
qsw->sbar.seeme = see_scrollbar;
qsw->sbar.feelme = rt_feel_scrollbar;
qsw->sbar.group = &qsw->scroll;

/* finish up list area */
qsw->slist.x = qsw->uparr.x + qsw->uparr.width + IBD;
qsw->slist.y = qsw->uparr.y;
qsw->slist.seeme = see_scroll_names;
qsw->slist.feelme = feel_scroll_cels;
qsw->slist.group = &qsw->scroll;

/* finish up cursel (string field) area */
rightoff = qsw->slist.x + qsw->slist.width + IWD;
qsw->cursel.x = rightoff + ((rightwid - qsw->cursel.width)>>1);
qsw->cursel.y = qsw->slist.y;
qsw->cursel.height = font_height + 4;
qsw->cursel.seeme = see_string_req;
qsw->cursel.feelme = accept_name;
qsw->cursel.datme = &qsw->stq;
qsw->cursel.key_equiv = '\t';

/* do ok and cancel */
qsw->okb.width = qsw->canb.width = okcanmax;
qsw->okb.height = qsw->canb.height = font_height*4/3;
qsw->okb.x = qsw->canb.x = rightoff + ((rightwid - okcanmax)>>1);
qsw->okb.y = qsw->mh.height - 10*font_height/3;
qsw->canb.y = qsw->mh.height - 5*font_height/3;
qsw->okb.seeme = qsw->canb.seeme = dcorner_text;
qsw->okb.datme = ok;
qsw->canb.datme = cancel;
qsw->okb.feelme = mb_close_ok;
qsw->canb.feelme = mb_close_cancel;

/* initialize scroller structure */
qsw->scroll.names = items;
qsw->scroll.scroll_sel = &qsw->sbar;
qsw->scroll.list_sel = &qsw->slist;
qsw->scroll.font = font;
qsw->scroll.top_name = *ipos;
qsw->scroll.cels_per_row = 1;
qsw->scroll.feel_1_cel = feel_1_scroll;

/* for all buttons link up buttons and set flags */
for (i=0;; i++)
{
	qsw->buts[i].flags = MB_NORESCALE;
	if(i >= Array_els(qsw->buts)-1) /* last one's next stays NULL */
		break;
	qsw->buts[i].next = &qsw->buts[i+1];
}

init_name_scroller(&qsw->scroll,s);

/* finish up menu header */
qsw->mh.mbs = &(qsw->cursel); /* note string field is first one */
qsw->mh.ioflags = (MBPEN|MBRIGHT|KEYHIT); /* any of this */
qsw->mh.flags = MENU_NORESCALE;
qsw->mh.seebg = seebg_white;
menu_to_reqpos(s,&(qsw->mh));

*pmh = (Menuhdr *)qsw;
return(Success);
}

void cleanup_qscroller(Menuhdr *qc, SHORT *ipos)
{
Qscrollwork *qsw = (Qscrollwork *)qc;

*ipos = qsw->scroll.top_name;
pj_free(qsw);		/* qc == gqsw here if all is well */
gqsw = NULL;		/* defensive programming */
}

