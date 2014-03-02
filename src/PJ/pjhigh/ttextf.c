#include <stdarg.h>
#include "errcodes.h"
#include "pjbasics.h"
#include "menus.h"
#include "rastext.h"
#include "reqlib.h"
#include "ftextf.h"

static Wndo *topwin = NULL;
static int tried;
static SHORT top_test;
static SHORT bot_test;

void cleanup_toptext(void)
{
	close_wndo(topwin);
	topwin = NULL;
	tried = 0;
}
void check_top_wndo_pos()

/* see if cursor is close to covering window. move window if so */
{
Rectangle newpos;

	if (topwin == NULL)
		return;
	if(icb.sy < top_test) /* if over window and on top put on bottom */
	{
		if(topwin->y == 0) 
		{
			copy_rectfields(topwin,&newpos);
			newpos.y = (vb.screen->wndo.height - topwin->height);
			reposit_wndo(topwin,&newpos,NULL);
		}
	}
	else if(icb.sy > bot_test)
	{
		if(topwin->y != 0)
		{
			copy_rectfields(topwin,&newpos);
			newpos.y = 0;
			reposit_wndo(topwin,&newpos,NULL);
		}
	}
}
static Errcode open_topwin(Vfont *f)
{
WndoInit wi;
Errcode err;

	if(topwin)
		return(0);
	if(tried)
		return(Err_nogood);

	/* try for best colors, no takeover */
	try_mucolors(vb.screen);

	clear_mem(&wi, sizeof(wi)); /* start cleared x = y = 0 */
	wi.screen = vb.screen;   
	wi.width = 54*widest_char(f);	/* this will be full width in low res */
	wi.height = font_cel_height(f);
	sclip_rect((Rectangle *)&(wi.RECTSTART),
	           (Rectangle *)&(vb.screen->wndo.RECTSTART));
	top_test = wi.height * 3;
	bot_test = vb.screen->wndo.height - top_test;
	wi.flags = (WNDO_NOCLEAR); 

	if((err = open_wndo(&topwin,&wi)) < 0)
		goto error;

	topwin->ioflags = MMOVE;
	topwin->doit = check_top_wndo_pos;

error:
	tried = 1;
	return(err);
}

Errcode ttextf(char *fmt,va_list argptr,char *formats)

/* formats and write text across top of screen. Formatted text cant exceed
 * sizeof buff[] or else !! */
{
int spwid;
char buff[TTEXTF_MAXCHARS];
Vfont *f = vb.screen->mufont;
int yoset;
Errcode err;

	if((err = open_topwin(f)) < 0)
		return(err);

	spwid = fchar_spacing(f,"m");
	vnsftextf(buff,sizeof(buff),formats,fmt,argptr);
    yoset = font_ycent_oset(f,topwin->height);
	pj_set_rast(topwin,swhite);

	check_top_wndo_pos();

	return(gftext(topwin,f,buff,
		   spwid, font_ycent_oset(f,topwin->height),
	   	   vb.screen->SBLACK, TM_MASK2, vb.screen->SWHITE));
}

