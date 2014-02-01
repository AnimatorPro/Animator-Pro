/* textbox.c - Stuff for continue alerts and yes/no dialogs. */

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

extern char *any_continue;
extern char *enter_choice;

/***** text boxes: these are at a lower level than windows so are used for
 	   error alerts and the like. they disable all input "wtasks" while they
	   are up. The Multi box case could be converted to a window but the 
	   yes no and continue box should be (error requestors independant of
	   the windows *******/

typedef struct tbclicker {
	RECT_FIELDS;
	SHORT id;
	SHORT tx, ty; /* text x and y offset */
	char *text;
	struct tbclicker *next;
} Tbclicker;

typedef struct tbox {
	char *text;
	va_list args;
	char *extratext;  /* bottom text */
	UBYTE rastopen;
	UBYTE reset_colors;
	Wiostate ios;
	Raster r;
	Tbclicker *clicks;
	Wscreen *tbs;
	char *formats;
	UBYTE mouse_was_up;
} Tbox;

void enable_textboxes()
{
	icb.wflags |= IWF_TBOXES_OK;
}
int disable_textboxes()
{
int ret;

	ret = icb.wflags & IWF_TBOXES_OK;
	icb.wflags &= ~(IWF_TBOXES_OK);
	return(ret);
}
static void init_tbox(Tbox *tbox,Wscreen *s,
					  char *formats,char *text,va_list args,char *extratext)
{
	clear_struct(tbox);
	tbox->text = text;
	tbox->extratext = extratext;
	copy_va_list(args,tbox->args);
	tbox->tbs = s;
	tbox->formats = formats;
}

static void diag_line(void *rast, int color, int dots,
	int x, int y, int dx, int dy)
{
while (--dots>=0)
	{
	pj_put_dot(rast,color,x,y);
	x += dx;
	y += dy;
	}
}

void box_diag_corner(void *rast,int x,int y,int w,int h,int color,int bevel)
{
int x1 = x+w-1;
int y1 = y+h-1;
int b2 = bevel*2;
int bm = bevel-1;
int bp = bevel+1;

	x1 = x+w-1;
	y1 = y+h-1;
	pj_set_hline(rast, color, x+bevel, y, w-b2);
	pj_set_hline(rast, color, x+bevel, y1, w-b2);
	pj_set_vline(rast, color, x, y+bevel, h-b2);
	pj_set_vline(rast, color, x1, y+bevel, h-b2);
	if (bm > 0)
		{
		diag_line(rast,color,bm,x+1,y+bevel-1,1,-1);
		diag_line(rast,color,bm,x1-1,y+bevel-1,-1,-1);
		diag_line(rast,color,bm,x+1,y1-bevel+1,1,1);
		diag_line(rast,color,bm,x1-1,y1-bevel+1,-1,1);
		}
}

static void drawcpyr(void *rast, 
	int color, int count, int x, int y, int w, int dy)
/* draw a cut off pyramid */
{
while (--count >= 0)
	{
	pj_set_hline(rast,color,x,y,w);
	w-=2;
	x+=1;
	y+=dy;
	}
}

static void draw_oct(void *rast,int x,int y,int w,int h,int color, int bevel)
/* draw an octagon.  bevel is the length of diagonal sides. */
{
	drawcpyr(rast, color, bevel, x+1, y+bevel-1, w-2, -1);
	pj_set_rect(rast,color,x, y+bevel, w, h-2*bevel);
	drawcpyr(rast, color, bevel, x+1, y+h-bevel, w-2, 1);
}

void diag_inside(void *rast,int x,int y,int w,int h,int color, int bevel)
{
	draw_oct(rast,x+1,y+1,w-2,h-2,color,bevel-1);
}
static LONG tblock_width(Vfont *f,char **tblock)

/* returns max text line width */
{
char *tline;
LONG thiswid;
LONG width = 0;

	while((tline = *tblock++) != NULL)
	{
		if((thiswid = fstring_width(f,tline)) > width)
			width = thiswid;
	}
	return(width);
}
static int build_tbclicks(Tbox *tbox,register Tbclicker *clicks, 
								 char **tblock)

/* makes a bunch of equal sized and equal distant clickers 
 * note the last one will have an id equal to the number of clickers 
 * returns number of clickers built */
{
Vfont *f = tbox->tbs->mufont;
char *text;
int height;
int width;
UBYTE ty;
Tbclicker *first = NULL;
SHORT id = 0;

	if(tblock == NULL)
		return(0);

	height = (9*font_cel_height(f))/5; /* 9/5 */
	ty = font_ycent_oset(f,height);
	width = tblock_width(f,tblock) + ((5*fchar_spacing(f," "))/2);

	while((text = *tblock++) != NULL)
	{
		clicks->id = ++id;
		clicks->text = text;
		clicks->height = height;
		clicks->width = width;
		clicks->ty = ty;
		clicks->tx = (width>>1) - (fstring_width(f,text)>>1);
		clicks->next = first;
		first = clicks;
		++clicks;
		if(id > TBOX_MAXCHOICES)
			break;
	}
	tbox->clicks = first;
	return(id);
}
static int space_tbclicks(Tbox *tbox, SHORT min_between)

/* spreads out clickers and sets x values to add clickers to bottom of tbox
 * will extend tbox in width and height if required to fit clickers
 * returns ammount width needs to be extended if any */
{
int numclicks;
Tbclicker *clicks;
int width;
int dx;
int x;
int y;

	clicks = tbox->clicks;
	numclicks = clicks->id; 
	width = (numclicks * (min_between + clicks->width)) - min_between; 
	if(width < tbox->r.width)
		width = tbox->r.width;
	dx = width/numclicks;
	x = (width >> 1) + (((dx * numclicks) - dx - clicks->width) >> 1);
	y = tbox->r.height;
	tbox->r.height += clicks->height;
	while(clicks) /* list is reverse of original text block */	
	{
		clicks->x = x;
		clicks->y = y;
		x -= dx;
		clicks = clicks->next;
	}
	return(width - tbox->r.width);
}
static void draw_oset_clicks(Tbox *tbox, SHORT xoset)

/* adds xoset to clicks x value and draws clicks at positino of tbox */
{
Tbclicker *clicks;
Wscreen *s = tbox->tbs;
int y;
int x;

	clicks = tbox->clicks;
	y = tbox->r.y + clicks->y;

	while(clicks)
	{
		clicks->x += xoset;
		x = clicks->x + tbox->r.x;

		diag_inside(s->viscel,x,y,clicks->width,clicks->height, s->SBLACK,
			s->bbevel);
		box_diag_corner(s->viscel,x,y, clicks->width, clicks->height,
						s->SGREY,s->bbevel);

		gftext(s->viscel,s->mufont,clicks->text,
			   x + clicks->tx, y + clicks->ty,
			   s->SWHITE,TM_MASK1);

		clicks = clicks->next;
	}
}
static int wait_poll_clicks(Tbox *tbox)

/* waits until user clicks in a clicker box or hits 
 * key of first text character returns id of clicker or 0 if right mouse
 * button was hit */
{
int c;
Tbclicker *clicks;
SHORT x,y;

	for (;;)
	{
		wait_input(MBPEN|MBRIGHT|KEYHIT);

		if(JSTHIT(KEYHIT))
		{
			c = toupper((UBYTE)icb.inkey);
			clicks = tbox->clicks;
			while(clicks)
			{
				if(c == toupper(clicks->text[0]))
					return(clicks->id);
				clicks = clicks->next;
			}
		}

		if(JSTHIT(MBPEN))
		{
			x = icb.sx - tbox->r.x;
			y = icb.sy - tbox->r.y;
			clicks = tbox->clicks;
			while(clicks)
			{
				if(ptin_rect((Rectangle *)&(clicks->RECTSTART),x,y))
					return(clicks->id);
				clicks = clicks->next;
			}
		}
		if(JSTHIT(MBRIGHT))
			return(0);

	}
}
static remclose_tbox(Tbox *tbox)
{
	if(tbox->rastopen)
	{
		pj_blitrect(&(tbox->r),0,0,tbox->tbs->viscel,tbox->r.x,tbox->r.y,
				 tbox->r.width,tbox->r.height);
		pj_close_raster(&(tbox->r));
		tbox->rastopen = 0;
	}
	wait_penup();
	rest_wiostate(&(tbox->ios));
	if(tbox->reset_colors && tbox->tbs)
		uncheck_mucmap(tbox->tbs);
	enable_wtasks(); /* ahhh.. back to normal */
	if(!tbox->mouse_was_up)
		hide_mouse();
}
static Errcode openput_tbox(Tbox *tbox)

/* returns Success if all ok ecode on fail */
{
Errcode err;
SHORT cheight;
SHORT cwidth;
SHORT vbord;    /* vertical border height */
SHORT txbord;	/* textblock border width */
SHORT clickoset;	/* clicker border x offset width */
SHORT txtwidth;
Wscreen *tbs;
LONG extray;
char sbuf[256];
char *tbuf;
char *text;

	if((tbs = tbox->tbs) == NULL)
		return(Err_bad_input);

	tbox->mouse_was_up = show_mouse();
	text = tbox->text;

	check_push_icb(); /* may be called in recursive input */
	disable_wtasks(); /* nope. suspend them, take over */
	tbuf = sbuf;
	save_wiostate(&(tbox->ios));
	load_wndo_iostate(NULL);

	/* make sure swhite etc are set to valid values */

	if(!(tbs->flags & WS_MUCOLORS_UP))
		tbox->reset_colors = 1;

	find_mucolors(tbs); 
	copy_rasthdr(tbs->viscel,&tbox->r);

	/* get font based character dimensions */

	cheight = font_cel_height(tbs->mufont);
	cwidth = fchar_spacing(tbs->mufont," ");
	vbord = cheight + cheight/3;
	txbord = cwidth + cwidth/3;

	/* if present add size of text block */

	tbox->r.height = vbord;
	if(text != NULL) /* no text */
	{
		if((err = get_formatted_ftext(&tbuf,sizeof(sbuf),
								      tbox->formats,text,tbox->args,
									  FALSE )) < Success)
		{
			goto error;
		}
		if(tbuf != NULL)
			text = tbuf;

		tbox->r.height += (cheight * wwcount_lines(tbs->mufont,text,
					     (tbs->wndo.width/4) * 3,(SHORT *)&tbox->r.width));
		tbox->r.height += vbord; /* add bottom border */
	}
	else
		tbox->r.width = 0;

	if(tbox->extratext)
	{
		extray = tbox->r.height;
		if(tbox->text)
			extray += cheight - vbord;

		tbox->r.height += cheight * (1 + wwcount_lines(tbs->mufont,
					tbox->extratext, (tbs->wndo.width/4) * 3,&txtwidth));

		if(txtwidth > tbox->r.width)
			tbox->r.width = txtwidth;
	}


	/* if present add size for clickers */

	if(tbox->clicks != NULL)
	{
		clickoset = txbord;
		txbord += (space_tbclicks(tbox, txbord) >> 1);
		tbox->r.height += vbord;
	}
	tbox->r.width += (txbord * 2);

	/* center box over cursor position and clip to screen */

	get_requestor_position(tbs,tbox->r.width,tbox->r.height,
						   (Rectangle *)&(tbox->r.RECTSTART));	

	/* save screen area behind box */

	if((err = pj_open_bytemap((Rasthdr *)&(tbox->r),(Bytemap *)&(tbox->r)))<0)
		goto error;
	tbox->rastopen = 1;

	pj_blitrect(tbs->viscel,tbox->r.x,tbox->r.y,&(tbox->r),
			 0,0,tbox->r.width,tbox->r.height);

	/* fill rectangle with solid color */

	pj_set_rect(tbs->viscel,tbs->SWHITE,
			 tbox->r.x,tbox->r.y,tbox->r.width,tbox->r.height );

	/* draw border around whole box */

	draw_quad(tbs->viscel,tbs->SGREY, tbox->r.x, tbox->r.y, 
						     tbox->r.width, tbox->r.height);

	/* draw textblock with border offsets */

	if(text != NULL)
	{
		wwtext(tbs->viscel,tbs->mufont,text,
				tbox->r.x + txbord, tbox->r.y + vbord,
				tbox->r.width - (txbord<<1),tbox->r.height,
				0, JUST_LEFT, tbs->SBLACK, TM_MASK1, 0);

	}

	if(tbox->extratext != NULL)
	{
		wwtext(tbs->viscel,tbs->mufont,tbox->extratext,
				tbox->r.x + txbord, tbox->r.y + extray,
				tbox->r.width - (txbord<<1),tbox->r.height,
				0, JUST_CENTER, tbs->SBLACK, TM_MASK1, 0);
	}

	if(tbox->clicks != NULL)
		draw_oset_clicks(tbox,clickoset);

	err = Success;
	goto done;

error:
	remclose_tbox(tbox);
done:
	if(tbuf != sbuf)
		pj_freez(&tbuf);
	return(err);
}
#define LBORD "\n\xB3 "  /* string for bios for vertical line formation */
static void print_topbord()
{
int i;

	fprintf(stderr, "\n\xda");
	i = 35;
	while(i-- > 0)
		fputc( 0xc4 ,stderr);
}
static Errcode stdout_tbox_start(Tbox *tbox)
{
Errcode err;
Ftextfarg fa;
char c, *txt;

	print_topbord();
	fprintf(stderr, LBORD);

	if((err = init_eitherfarg(&fa,tbox->formats,tbox->text)) < Success)
	{
		printf("error %d loading |%s|\n|%s|\n",err,tbox->formats,tbox->text);
		getchar();
		return(err);
	}
	copy_va_list(tbox->args,fa.fa.args);

	while(c = fa_getc(&fa.fa))
	{
		if(c == '\n')
			fprintf(stderr, LBORD);
		else
			fputc(c,stderr);
	}
	if((txt = tbox->extratext) != NULL)
	{
		c = '\n';
		while(c != 0)
		{
			if(c == '\n')
				fprintf(stderr, LBORD);
			else
				fputc(c,stderr);
			c = *txt++;
		}
	}
	return(Success);
}
static int stdout_wait_any()
{
	fprintf(stderr, LBORD LBORD "%s", any_continue );
	fflush(stderr);
	dos_wait_key();
	fputc('\n', stderr);
	return(0);
}
Errcode tboxf(Wscreen *s,char *text,va_list args)

/* puts up a textbox and waits on an input key hit bypassing input routines 
 * doesn't display any buttons or use mouse */
{
Errcode err;
Tbox tbox;

	init_tbox(&tbox,s,NULL,text,args,NULL);
	if(!(icb.wflags & IWF_TBOXES_OK) || s == NULL)
	{
		if((err = stdout_tbox_start(&tbox)) >= Success)
			err = stdout_wait_any();
	}
	else if((err = openput_tbox(&tbox)) >= Success)
	{
		dos_wait_key();
		remclose_tbox(&tbox);
	}
	return(err);
}
Errcode tboxf_choice(Wscreen *s,char *formats,char *text,va_list args,
					 char **choices, char *extratext)

/* given a null terminated array of choices will return 0 if canceled
 * or error and 1 for the first choice TBOX_MAXCHOICES */
{
Tbox tbox;
Tbclicker clicks[TBOX_MAXCHOICES];
Errcode err, ret;
char *choice;
int i;
char in_c;


	init_tbox(&tbox,s,formats,text,args,extratext);

	if(!(icb.wflags & IWF_TBOXES_OK) || s == NULL)
	{
		/**** do it the stdout, wait key way ****/

		for(;;) /* until they get it keep re-displaying */
		{
			if((err = stdout_tbox_start(&tbox)) < Success)
				return(err);

			for(i = 0;((choice = choices[i]) != NULL); ++i);

			if(i < 2)
				return(stdout_wait_any());

			fprintf(stderr, LBORD);
			for(i = 0;((choice = choices[i]) != NULL); ++i)
				fprintf(stderr, LBORD "'%c' - %s", choice[0], choice);

			fprintf(stderr, LBORD LBORD "%s", enter_choice );
			fflush(stderr);
			in_c = toupper((UBYTE)dos_wait_key());
			for(i = 0;((choice = choices[i]) != NULL);)
			{
				++i;
				if(in_c == toupper(choice[0]))
				{
					fputc('\n', stderr);
					return(i);
				}
			}
		}
	}

	ret = build_tbclicks(&tbox,clicks,choices);
	if((err = openput_tbox(&tbox)) < 0)
		return(err);
	wait_penup();
	if(ret > 1)
	{
		ret = wait_poll_clicks(&tbox);
	}
	else
	{
		wait_click();
		ret = 0;
	}
	remclose_tbox(&tbox);
	return(ret);
}
