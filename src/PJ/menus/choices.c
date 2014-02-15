
/* choices.c - build up simple list of selections type menus out of
   a list of strings.  */

#include <stdio.h>
#include <string.h>
#include "errcodes.h"
#include "menus.h"
#include "rastext.h"
#include "input.h"
#include "memory.h"
#include "ptrmacro.h"
#include "commonst.h"

typedef struct choicer {
	Menuhdr mh;
	VFUNC do_on_hit;
	Button buttons[1];
} Choicer;

static void hide_on_choice(Button *b)
{
	if(b->identity != 0)
		mb_hide_menu(b);
}

/* This module builds up a menu from a 'header' and a list of choices, all
   of which are simple ascii text strings */

#define CMAX 10

static int do_choicemenu(Menuhdr *hdr)
/* function for the domenu function choice requestor */
{
int ret;
Button *hit;
Button *mbs;
Button *last_hilit = NULL;

	for(;;) /* take over until mouse exits menu */
	{
		mbs = ((Button *)hdr->mbs)->next; /* note: mbs->next is first choice */ 
		if(JSTHIT(MMOVE))
		{
			if(last_hilit != (hit = hit_button(mbs,icb.mx,icb.my)))
			{
				safe_mc_frame(last_hilit,MC_WHITE);
				if (hit != NULL && !(hit->flags & MB_DISABLED))
					safe_mc_frame(hit,MC_RED);
				last_hilit = hit;
			}
		}

		if(0 != (ret = button_keyhit(hdr,hdr->mbs,
									((Choicer *)hdr)->do_on_hit )))
		{
			safe_mc_frame(last_hilit,MC_WHITE); 
			if(ret == 2) /* ate key */
				return(1);
			last_hilit = NULL;
		}
		else if(2 == check_reqabort(hdr))
			return(1); /* ate key */
		
		/* out of menu? go back. Note: will be false if hidden or closed */
		if(!cursin_menu(hdr)) 
			return(ret); /* button ate something? */
		wait_input(hdr->ioflags);
	}
}
static void see_choice_title(Button *b)
/* see titlebar, but stop at a newline or null */
{
char *send;

	if(NULL != (send = strchr(b->datme,'\n')))
		*send = 0;
	see_titlebar(b);
	if(send)
		*send = '\n';
}
static void see_choice(Button *b)
{
char buff[100];
char *send;
int x, y;
Vfont *f = b->root->font;
Pixel color;

	/* this will terminate choice string with either a newline or a null */
	if(NULL != (send = strchr(b->datme,'\n')))
		*send = 0;
	mc_block(b,MC_WHITE);

	x = b->x + (fchar_spacing(f," ")>>1) + 1,
	y = b->y + font_ycent_oset(f,b->height),

	sprintf(buff, "%c %s", b->key_equiv, b->group);
	color = wbg_textcolor(b);
	gftext(b->root,f,buff,x,y,color,TM_MASK1 );
	x += fstring_width(f,"9 *");
	gftext(b->root,f,b->datme,x,y,color,TM_MASK1 );
	if(send)
		*send = '\n';
}
static void auto_selectid(Button *b)
{
	mb_close_code(b,b->identity);
}
void cleanup_qchoice(Menuhdr *mh)
{
	if(mh)
		pj_free(mh);
}

Errcode build_qchoice(Wscreen *s, Menuhdr **pmh, char *header, char **choices, 
				  	 int ccount, VFUNC *feelers, Boolean hide_on_hit,
					 USHORT *flags)

/*** this allocates and builds a multi choice menu and its buttons and does 
 ** all preparation needed before open_menu() is to be called on it 
 ** cleanup_qchoice() must be used to free it. All choices are terminated with 
 ** newlines or '\0' chars, The input strings are not altered, The last item
 ** will not be asterisked of disabled if there are feelers ***/
{
Choicer *ch;
Vfont *f;
SHORT bheight;  /* choice button height */
SHORT hdrht;	/* header height */
SHORT hdrwid;
Button *b;
SHORT by;
int i;
int flag;
char *choice;

	if (ccount > CMAX)	/* defensive programming */
		ccount = CMAX;

	if(NULL == (*pmh = pj_zalloc( PADSIZE(Choicer)
							    + (PADSIZE(Button)*(ccount)))))
	{
		return(Err_no_memory);
	}

	ch = (Choicer *)(*pmh);
	b = &ch->buttons[0];
	ch->mh.mbs = b;

	f = s->mufont;
	hdrht = bheight = font_cel_height(f) + 1;
	bheight += 2;  /* for border */ 

	ch->mh.width = widest_line(f,choices,ccount) + fstring_width(f," 9 *")+1;
	if((hdrwid = fline_width(f,header) + fchar_spacing(f," ") + 2*hdrht) 
		> ch->mh.width)
	{
		ch->mh.width = hdrwid;
	}
	ch->mh.width += 2;	/* for border */	

	/* load header button fields */

	b->x = 1;
	b->width = ch->mh.width - 2; /* x,y at 0,0 top */
	b->height = hdrht;
	b->seeme = see_choice_title;
	b->feelme = feel_req_titlebar;
	b->optme = mb_menu_to_bottom;
	b->group = &tbg_moveclose;
	b->datme = header;
	b->next = b+1;
	b->flags = MB_NORESCALE;
	by = hdrht;

	for(i = 0;i < ccount;++i)
	{
		++b;
		choice = *choices++;
		b->group = space_str;
		if (flags != NULL)
			{
			flag = *flags++;
			if (flag & QCF_DISABLED)
				b->flags = MB_DISABLED;
			if (flag & QCF_ASTERISK)
				b->group = "*";
			}
		b->y = by;
		b->flags |= MB_NORESCALE;
		b->identity = i;
		b->height = bheight; 
		by += bheight - 1;
		b->x = 1;
		b->width = ch->mh.width - 2;
		b->key_equiv = '1' + i;
		b->datme = choice; 
		b->seeme = see_choice;
		if(feelers != NULL)
			b->feelme = *feelers++;
		else
			b->feelme = auto_selectid;
		b->next = b+1;
	}
	b->next = NULL;
	if(ccount)
	{
		if(!feelers) /* don't disabel the cancle button if default feelers! */
		{
			b->flags &= ~(MB_DISABLED);
			b->group = space_str;
		}
		b->identity = Err_abort;
		b->key_equiv = '0';
	}

	if(hide_on_hit)
		ch->do_on_hit = hide_on_choice;
	ch->mh.height = b->y + b->height + bheight/2; /* hieght beyond last btn */
	ch->mh.ioflags = (MMOVE|MBPEN|MBRIGHT|KEYHIT); /* any of this */
	ch->mh.flags = MENU_NORESCALE;
	ch->mh.domenu = do_choicemenu; /* the picker */
	ch->mh.seebg = seebg_white;
	menu_to_reqpos(s,&ch->mh);
	return(0);
}

