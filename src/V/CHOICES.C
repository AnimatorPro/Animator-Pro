
/* choices.c - build up simple list of selections type menus out of
   a list of strings.  */

#include "jimk.h"
#include "flicmenu.h"

extern gary_menu_back(), gary_see_title(),blacktext(),
	move_menu(), bottom_menu();
/* This module builds up a menu from a 'header' and a list of choices, all
   of which are simple ascii text strings */

#define CMAX 10

typedef struct qccb 
	{
	int chosen;
	Flicmenu choicesels[CMAX];
	Flicmenu choiceheader;
	Flicmenu choicemenu;
	} Qccb;


static
accept_choice(m)
Flicmenu *m;
{
*(m->group) = m->identity;
close_menu();
}

static
see_choice(m)
Flicmenu *m;
{
char buff[60];

sprintf(buff, "%c %s", m->key_equiv, m->text);
gtext( buff,  m->x,  m->y+1,  sblack);
}

#define TH (CH_HEIGHT+2)

static Qccb *make_qchoice(header, choices, ccount, feelers)
char *header;
char **choices;
int ccount;
Vector *feelers;
{
int i;
int cwidth, cheight, cy, cx;
Flicmenu *mp;
Qccb *qcb;

if ((qcb = begmemc(sizeof(*qcb)))  == NULL)
	return(NULL);
if (ccount > CMAX)	/* defensive programming */
	ccount = CMAX;

/* Calculate dimensions of underlying menu */
/* 1 for each choice, one for title, and one for border */
cheight = ccount*TH + CH_HEIGHT + CH_HEIGHT;
/* Find longest choice */
cwidth = longest_string(choices, ccount)+3;/* space for number period space */
/* if header even longer use it... */
i = strlen(header);
if (i > cwidth)
	cwidth = i;
cwidth += 1;	/* add space for border */
cwidth *= CH_WIDTH;
cx = uzx-cwidth/2;
cy = uzy-cheight/2;

/* Initialize menu skeleton */
qcb->choiceheader.next = &(qcb->choicesels[0]);
qcb->choiceheader.seeme = gary_see_title;
qcb->choiceheader.height = CH_HEIGHT;
qcb->choicemenu.children =  &qcb->choiceheader;
qcb->choicemenu.seeme = gary_menu_back;

/* Initialize position of menu and header sel */
qcb->choicemenu.x = qcb->choiceheader.x = cx;
qcb->choicemenu.y = qcb->choiceheader.y = cy;
qcb->choicemenu.width = qcb->choiceheader.width = cwidth;
qcb->choicemenu.height = cheight;
qcb->choiceheader.text = header;

/* move x/y position to where choices will start */
cy += CH_HEIGHT + CH_HEIGHT/2;
cx += CH_WIDTH/2;

/* Now go make the choices */
mp = qcb->choicesels;
for (i=0; i<ccount; i++)
	{
	mp->next = mp+1;
	mp->x = cx;
	mp->y = cy;
	cy += TH;
	mp->width = cwidth;
	mp->height = TH-1;
	mp->text = choices[i];
	mp->seeme = see_choice;
	if (feelers != NULL)
		mp->feelme = *feelers++;
	else
		mp->feelme = accept_choice;
	mp->group = &qcb->chosen;
	mp->identity = i+1;
	mp->key_equiv = '1'+i;
	if (i == ccount-1)
		{
		mp->key_equiv = '0';
		mp->identity = 0;
		}
	mp++;
	}
/* make last next pointer show end of list */
mp -= 1;
mp->next = NONEXT;
clip_rmove_menu(&qcb->choicemenu,0,0);	/* just a clip, not a move usually */
return(qcb);
}

/* Pass this one a string for the title bar of menu, and an array of strings
   ccount long of choices  (make sure ccount less than 10).  This will
   put up a menu and return the item from that menu chosen.  Returns after
   one choice.  The last item in the 'choices' list is always numbered
   zero, and zero is returned if this is selected.  Zero return also on
   space bar or right click outside menu area. */
qchoice(header, choices, ccount)
char *header;
char **choices;
int ccount;
{
int chosen;
Qccb *qcb;

if ((qcb = make_qchoice(header, choices, ccount,NULL)) != NULL)
	{
	do_menu(&qcb->choicemenu);
	chosen =  qcb->chosen;
	freemem(qcb);
	return(chosen);
	}
}

/* Put up a numbered item menu that doesn't return until a 'exit menu' choice
   is made.  First three parameters are as for qchoice above.  'feelers'
   parameter is a list of functions to be called when the corresponding
   choice is made.  */
qmenu(header, choices, ccount, feelers)
char *header;
char **choices;
int ccount;
Vector *feelers;
{
Qccb *qcb;

if ((qcb = make_qchoice(header, choices, ccount, feelers)) != NULL)
	{
	hfs_do_menu(&qcb->choicemenu);
	freemem(qcb);
	}
}

