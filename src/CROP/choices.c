
/* choices.c - put up a little menu of numbered choices. */

#include "jimk.h"
#include "flicmenu.h"

extern gary_menu_back(), gary_see_title(),blacktext(),
	move_menu(), bottom_menu();
/* This module builds up a menu from a 'header' and a list of choices, all
   of which are simple ascii text strings */

#define CMAX 10

static int chosen;
static struct flicmenu choicesels[CMAX];
static struct flicmenu choiceheader = {
	NOCHILD, 
	&choicesels[0],
	0,0,0,CH_HEIGHT,
	NOTEXT,
	gary_see_title,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static struct flicmenu choicemenu = {
	NOCHILD, 
	&choiceheader,
	0,0,0,0,
	NOTEXT,
	gary_menu_back,
	NOFEEL,
	NOGROUP,0,NOKEY,NOOPT,
	};

accept_choice(m)
Flicmenu *m;
{
*(m->group) = m->identity;
close_menu();
}

see_choice(m)
Flicmenu *m;
{
char buff[60];

sprintf(buff, "%c %s", m->key_equiv, m->text);
gtext( buff,  m->x,  m->y+1,  sblack);
}

#define TH (CH_HEIGHT+2)

static
make_qchoice(header, choices, ccount, feelers)
char *header;
char **choices;
int ccount;
Vector *feelers;
{
int i;
int cwidth, cheight, cy, cx;
Flicmenu *mp;

if (ccount > CMAX)	/* defensive programming */
	ccount = CMAX;
chosen = 0;	/* Zero out return choice value (will always be >0 if a choice*/


/* Calculate dimensions of underlying menu */
/* 1 for each choice, one for title, and one for border */
cheight = ccount*TH + CH_HEIGHT + CH_HEIGHT;
/* Find longest choice */
cwidth = longest_string(choices, ccount)+3;	/* space for number period space */
/* if header even longer use it... */
i = strlen(header);
if (i > cwidth)
	cwidth = i;
cwidth += 1;	/* add space for border */
cwidth *= CH_WIDTH;
cx = uzx-cwidth/2;
cy = uzy-cheight/2;

/* Initialize position of menu and header sel */
choicemenu.x = choiceheader.x = cx;
choicemenu.y = choiceheader.y = cy;
choicemenu.width = choiceheader.width = cwidth;
choicemenu.height = cheight;
choiceheader.text = header;

/* move x/y position to where choices will start */
cy += CH_HEIGHT + CH_HEIGHT/2;
cx += CH_WIDTH/2;

/* Now go make the choices */
mp = choicesels;
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
	mp->group = &chosen;
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
clip_rmove_menu(&choicemenu,0,0);	/* just a clip, not a move usually */
}

qchoice(header, choices, ccount)
char *header;
char **choices;
int ccount;
{
make_qchoice(header, choices, ccount,NULL);
do_menu(&choicemenu);
return(chosen);
}

#ifdef SLUFFED
qmenu(header, choices, ccount, feelers)
char *header;
char **choices;
int ccount;
Vector *feelers;
{
make_qchoice(header, choices, ccount, feelers);
hfs_do_menu(&choicemenu);
}
#endif /* SLUFFED */

