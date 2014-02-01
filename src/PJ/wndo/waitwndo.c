/* waitwndo.c - Stuff for please wait window */

#include <stdarg.h>
#include "errcodes.h"
#include "ftextf.h"
#include "gfx.h"
#include "input.h"
#include "lstdio.h"
#include "memory.h"
#include "rastext.h"
#include "wndo.h"
#include "wordwrap.h"

/************* the "Please Wait" window *************/
void cleanup_wait_wndo(Wscreen *s)
{
	if(s->wait_box != NULL)
	{
		rem_waitask((Waitask *)(s->wait_box + 1));
		close_wndo(s->wait_box);
		s->wait_box = NULL;
	}
}
static Boolean autorem_wait_wndo(Waitask *wt)
{
	cleanup_wait_wndo((Wscreen *)(wt->data));
	return(1);
}
static Errcode open_waitwin(Wscreen *screen, Rectangle *box)
{
Errcode err;
Waitask *wt;
WndoInit wi;

	clear_mem(&wi,sizeof(wi));
	wi.width = box->width;
	wi.height = box->height;
	wi.x = (screen->wndo.width - wi.width)>>1;
	wi.y = (screen->wndo.height - wi.height)>>1;
	wi.screen = screen;
	wi.flags = WNDO_NOCLEAR;
	wi.extrabuf = sizeof(Waitask);
	if((err = open_wndo(&screen->wait_box,&wi)) >= Success)
	{
		wt = (Waitask *)(screen->wait_box + 1);
		init_waitask(wt,autorem_wait_wndo,screen,WT_KILLCURSOR);
		add_waitask(wt);
	}
	return(err);
}
Errcode va_wait_wndo(Wscreen *screen, char *wait_str, 
					 char *formats, char *text, va_list args)
{
Errcode err;
char sbuf[256];
char *tbuf;
Rectangle box;
SHORT lineht, cwid, maxwid;
Vfont *f;

	if(!(icb.wflags & IWF_TBOXES_OK) || screen == NULL)
		return(Success);

	cleanup_wait_wndo(screen);

	tbuf = sbuf;
	if((err = get_formatted_ftext(&tbuf,sizeof(sbuf),
								  formats,text,args,FALSE)) < Success)
	{
		return(err);
	}
	if(tbuf)
		text = tbuf;

	/* get size of wordwrap text block */

	f = screen->mufont;
	lineht = font_cel_height(f);
	cwid = fchar_spacing(f,"M");
	maxwid = (screen->wndo.width<<1)/3 - (cwid<<1);
	box.height = wwcount_lines(f,text,maxwid,(SHORT *)&box.width);
	if(box.width < fstring_width(f,wait_str))
		box.width = fstring_width(f,wait_str);

	box.width += (cwid<<1);
	box.height += 3;
	box.height *= lineht;
	box.y = (lineht*2)/3;
	box.height += box.y;

	if((err = open_waitwin(screen,&box)) < Success)
		goto error;

	pj_set_rast(screen->wait_box,screen->SWHITE);
	draw_quad(screen->wait_box,screen->SGREY,0,0,box.width,box.height);

	box.x = cwid;
	box.width -= (cwid<<1);
	justify_line(screen->wait_box,f,wait_str,box.x,box.y,box.width,
				 screen->SBLACK,TM_MASK1,0,JUST_CENTER,NULL,0);
	box.y += (lineht<<1);

	wwtext(screen->wait_box,f,text,box.x,box.y,box.width,box.height,
		   0, JUST_LEFT, screen->SBLACK, TM_MASK1,0);

	err = Success;
error:
	if(tbuf != sbuf)
		pj_freez(&tbuf);
	return(err);
}


