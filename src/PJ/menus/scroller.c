/* scroller.c - display scrolling name list in file selector, and help out
   browse menu a bit too.  Other scroller users are the draw tools and
   ink types menus.  */

#include "jimk.h"
#include "scroller.h"

#define GET_CEL_X(s,ix) (((ix)%s->cels_per_row)*(s->cel_width))
#define GET_CEL_Y(s,ix) (((ix)/s->cels_per_row)*(s->cel_height))

static int clip_scroll(Name_scroller *scroll, int newtop);

void get_scroll_cel_pos(Name_scroller *s, int ix, Short_xy *pos)

/* returns pixel position of scroller cel relative to the window scroller
 * is drawn on */
{
Button *b = s->list_sel;

	pos->x = GET_CEL_X(s,ix) + b->x + s->xoset;
	pos->y = GET_CEL_Y(s,ix) + b->y + s->yoset;
}

static void scroll_make_clip(Name_scroller *scroll, Clipbox *cbox)

/* make clipbos to represent scroller list_sel button drawing area */
{
Button *b = scroll->list_sel;

	 pj_clipbox_make(cbox, (Raster *)(b->root), 
	 				 b->x + scroll->xoset, b->y + scroll->yoset, 
					 b->width - scroll->xoset - scroll->border, 
					 b->height - scroll->yoset - scroll->border) ;
}
static void calc_scroll_pos(Name_scroller *scroll, Button *scroll_sel)
{
SHORT temp;
SHORT spare_height;
SHORT tname;

	scroll->yoff = 1;
	if ((temp = scroll->name_count - scroll->dcount) > 0)
	{
		tname = scroll->top_name;
		spare_height = scroll_sel->height - 2 - scroll->knob_height;
		if (tname > temp)	/* past end ?? */
			scroll->yoff += spare_height;
		else
			scroll->yoff += (spare_height*tname + (temp>>1))/temp ;
	}
}
void init_scroller(Name_scroller *scr,Wscreen *screen)
{
int remain;

	scale_button(scr->list_sel,&screen->menu_scale);
	scr->name_count = slist_len(scr->names);

	scr->xoset = scr->col_xoset + scr->border;
	scr->yoset = scr->row_yoset + scr->border;

	if(!scr->cels_per_col)
		scr->cels_per_col = 1;
	scr->cel_height = (scr->list_sel->height-(scr->border+scr->yoset))
													/scr->cels_per_col; 

	if(!scr->cels_per_row)
		scr->cels_per_row = 1;
	scr->cel_width = (scr->list_sel->width-(scr->border+scr->xoset))
													/scr->cels_per_row; 

	scr->dcount = scr->cels_per_row * scr->cels_per_col;

	if((scr->cliptest = scr->name_count - scr->dcount) > 0)
	{
		scr->endtop = scr->cliptest;
		if((remain = scr->name_count%scr->cels_per_row) > 0)
		{
			scr->cliptest -= remain;
			scr->endtop += scr->cels_per_row - remain;
		}
	}
	else
		scr->endtop = 0;

	scr->top_name = clip_scroll(scr,scr->top_name);

	/* init scroll sel constants and dimensions */

	scale_button(scr->scroll_sel,&screen->menu_scale);
	scr->scroll_sel->height -= 2; /* account for border */
	if (scr->name_count == 0)
		scr->knob_height = 0;
	else if (scr->name_count <= scr->dcount)
		scr->knob_height = scr->scroll_sel->height;
	else
	{
	static SHORT min200ht = 8;
	SHORT minht;

		scr->knob_height = 
			(scr->scroll_sel->height) * scr->dcount/scr->name_count;
		scale_ylist(&screen->menu_scale, &min200ht, &minht, 1);
		if (scr->knob_height < minht)
			scr->knob_height = minht;
	}
	scr->scroll_sel->height += 2; /* put border back */
	calc_scroll_pos(scr, scr->scroll_sel);
}

static void *the_big_feelme(Button *b, int *pwhy,
							void (*feel_1_cel)(Button *list_sel,
								  void *rast,int x,int y,Names *entry,
								  int why))
/* the big feel does it all It will identify what scroll cel the mouse is 
 * clicked over find the entry, check for double hits, call the hilite, 
 * the feelme, and the un-hilite It returns the element selected and 
 * the double hit Boolean for the other calls. pis_dhit and feel_1_cel
 * may be null and will be ignored */
{
static void *last_scroll;
static ULONG last_time; 
static void *last_entry;
int ix,x,y;
Name_scroller *scroll = (Name_scroller *)b->group;
Names *name;
Clipbox cbox;
ULONG time;
int why;

	time = pj_clock_1000();

	y = icb.my - b->y - scroll->yoset;
	if(y < 0)
		y = 0;
	x = icb.mx - b->x - scroll->xoset; 
	if(x < 0)
		x = 0;

	ix = ((y/scroll->cel_height) * scroll->cels_per_row)
			+ (x/scroll->cel_width);

	if(ix >= scroll->dcount)
		goto kill_dhit;

	x -= x%scroll->cel_width;
	y -= y%scroll->cel_height;

	name = slist_el(scroll->names, scroll->top_name + ix);

	if(!name)
		goto kill_dhit;

	scroll_make_clip(scroll,&cbox);

	why = ( scroll == last_scroll
				&& name == last_entry
				&& (time - last_time) < DHIT_MICROS)?SCR_MDHIT:SCR_MHIT;

	if(scroll->high_1_cel)
		(*scroll->high_1_cel)(b,&cbox,x,y,name,TRUE);

	wait_penup();

	if(scroll->high_1_cel)
		(*scroll->high_1_cel)(b,&cbox,x,y,name,FALSE);

	if(name != NULL && feel_1_cel)
		(*feel_1_cel)(b,&cbox,x,y,name,why);

	if(why == SCR_MDHIT)
	{
		last_scroll = NULL;
		last_entry = NULL;
	}
	else
	{
		last_entry = name;
		last_scroll = scroll;
		last_time = time;
	}
	goto OUT;

kill_dhit:
	scroll = NULL;
	name = NULL;
	why = SCR_MHIT;
OUT:
	if(pwhy)
		*pwhy = why;
	return(name);
}

void feel_scroll_cels(Button *b)
{
	the_big_feelme(b,NULL,((Name_scroller *)(b->group))->feel_1_cel);
}

static void blit_scroll_cel(Name_scroller *s, void *rast, 
							int src_ix, int dest_ix)

/* based on index in browse sel will blit the contents of the source sel to 
 * the dest sel does not clip to size */
{
	pj_blitrect(rast, GET_CEL_X(s,src_ix), GET_CEL_Y(s,src_ix), 
	         rast, GET_CEL_X(s,dest_ix), GET_CEL_Y(s,dest_ix), 
			 s->cel_width, s->cel_height); 
}
static void rescroll(Name_scroller *s)
/* calculate scroll bar position and redraw scroll button */
{
	calc_scroll_pos(s, s->scroll_sel);
	draw_buttontop(s->scroll_sel);
}
static void bscroll_up(Button *b, int count)

/* scroll up (window up over list) a scroller list blit moving cels that are
 * not redrawn and calling redraw function for ones to be refreshed */
{
Name_scroller *scroll = (Name_scroller *)(b->group);
int otop;
int obot;
int nbot;
Clipbox cbox;
Names *name;

	otop = scroll->top_name;
	scroll->top_name = clip_scroll(scroll,scroll->top_name - count);

	if(otop == scroll->top_name)
		return;

	/* get index+1 of last visible browse sel */

	obot = (scroll->name_count - scroll->top_name);
	if(obot > scroll->dcount)
		obot = scroll->dcount;

	/* new bottom sel is now above old bottom sel note this is referenced
	 * to 1 as the top index so we '--' in loop below */

	nbot = obot - (otop - scroll->top_name);

	scroll_make_clip(scroll,&cbox);

	while(--nbot >= 0)
		blit_scroll_cel(scroll,&cbox,nbot,--obot);

	/* this may be a little tiny bit slower but it looks better to have redraw 
	 * always the same direction */

	b = scroll->list_sel;
	name = slist_el(scroll->names, scroll->top_name);
	while(--obot >= 0)
	{
		(*scroll->draw_1_cel)(b,&cbox,GET_CEL_X(scroll,obot),
					    	    	  GET_CEL_Y(scroll,obot),
									  slist_el(name,obot));
	}
	rescroll(scroll);
}
static int clip_scroll(Name_scroller *scroll, int newtop)
{
	if(newtop < 0)
		return(0);
	if(newtop > scroll->cliptest)
		return(scroll->endtop);
	return(newtop);
}
static void bscroll_down(Button *b, int count)
{
Name_scroller *scroll = (Name_scroller *)(b->group);
int otop;
int ntop;
int obot;
Clipbox cbox;
Names *name;

	/* get index of new top (clipped count) old top is 0 */

	otop = scroll->top_name;
	ntop = clip_scroll(scroll,otop + count);
	if((ntop -= otop) <= 0)
		return;

	/* old last visible */

	if((obot = otop + scroll->dcount) > scroll->name_count)
		obot = scroll->name_count;
	obot -= scroll->top_name;

	scroll->top_name += ntop; /* set up by clipped count */
	scroll_make_clip(scroll,&cbox);

	otop = 0;
	while(ntop < obot)
		blit_scroll_cel(scroll,&cbox,ntop++,otop++);

	name = slist_el(scroll->names, scroll->top_name + otop);
	b = scroll->list_sel;
	while(otop < scroll->dcount)
	{
		(*scroll->draw_1_cel)(b,&cbox,GET_CEL_X(scroll,otop),
					    	    	  GET_CEL_Y(scroll,otop), name );
		++otop;
		if(name != NULL)
			name = name->next;
	}
	rescroll(scroll);
}

static void bincu(Button *b)
{
	bscroll_up(b, ((Name_scroller *)(b->group))->cels_per_row);
}
static void bincd(Button *b)
{
	bscroll_down(b, ((Name_scroller *)(b->group))->cels_per_row);
}
static void scroll_incit(Button *b, void *func)
{

#ifdef ARROW_KEY_ALTS
Name_scroller *scr;
SHORT end_name;
Clipbox cbox;
Names *name;
SHORT x,y,ix;

	scr = (Name_scroller *)(b->group);
	if(JSTHIT(MBPEN) || scr->no_key_mode)

#endif /* ARROW_KEY_ALTS */
	{
		hilight(b);
		repeat_on_pdn(func,b);
		draw_buttontop(b);
		return;
	}

#ifdef ARROW_KEY_ALTS

	if(!(JSTHIT(KEYHIT) && icb.inkey == DARROW || icb.inkey == UARROW))
		return;
	if(!scr->name_count)
		return;
	scroll_make_clip(scr,&cbox);

	for(;;)
	{
		if((end_name = scr->top_name + scr->dcount) > scr->name_count)
			end_name = scr->name_count;

		if(scr->key_name < scr->top_name)
			scr->key_name = scr->top_name;
		else if(scr->key_name >= end_name)
			scr->key_name = end_name-1;

		name = slist_el(scr->names, scr->key_name);
		ix = scr->key_name - scr->top_name;

		x = GET_CEL_X(scr,ix);
		y = GET_CEL_Y(scr,ix);

		if(scr->feel_1_cel)
			(*scr->feel_1_cel)(b,&cbox,x,y,name,SCR_ARROW);

		if(scr->high_1_cel)
			(*scr->high_1_cel)(b,&cbox,x,y,name,TRUE);

		wait_input(ANY_CLICK);

		if(scr->high_1_cel)
			(*scr->high_1_cel)(b,&cbox,x,y,name,FALSE);

		if(!JSTHIT(KEYHIT))
			break;

		switch(icb.inkey)
		{
			case LARROW:
				--scr->key_name;
				goto check_top;
			case UARROW:
				scr->key_name -= scr->cels_per_row;
			check_top:
				if(scr->key_name < scr->top_name)
					bincu(b);
				break;
			case RARROW:
				++scr->key_name;
				goto check_bot;
			case DARROW:
				scr->key_name += scr->cels_per_row;
			check_bot:
				if(scr->key_name >= end_name)
					bincd(b);
				break;
			default:
				if((UBYTE)icb.inkey == '\r' && scr->feel_1_cel)
				{
					(*scr->feel_1_cel)(b,&cbox,x,y,name,SCR_ENTER);
					return;
				}
				goto done;
		}
	}
done:
	reuse_input();
	return;
#endif /* ARROW_KEY_ALTS */

}
void scroll_incup(Button *b)
{
	scroll_incit(b,bincu);
}
void scroll_incdown(Button *b)
{
	scroll_incit(b,bincd);
}
void draw_scroll_cels(Button *b)

/* redraw all cels in a scroller calling the draw_1_cel function */
{
Name_scroller *scroll = (Name_scroller *)(b->group);
int x, y, ix, xix;
Names *name;
Clipbox cbox;

	b = scroll->list_sel;
	x = 0;
	y = 0;
	name = slist_el(scroll->names,scroll->top_name);
	scroll_make_clip(scroll,&cbox);
	xix = 0;
	for(ix = 0;ix < scroll->dcount;++ix)
	{
		(*scroll->draw_1_cel)(b,&cbox,x,y,name);
		if(name)
			name = name->next;
		if(++xix >= scroll->cels_per_row)
		{
			x = xix = 0;
			y += scroll->cel_height;
		}
		else
			x += scroll->cel_width;
	}
}
static void bpagedown(void *name_scroller)
{
	Name_scroller *scroll = name_scroller;

	scroll->top_name = clip_scroll(scroll, 
								scroll->top_name + scroll->dcount);
	scroll->top_name -= (scroll->top_name%scroll->cels_per_row);
	draw_scroll_cels(scroll->list_sel);
	rescroll(scroll);
}
static void bpageup(void *name_scroller)
{
Name_scroller *scroll = name_scroller;
int nt;

	nt = scroll->top_name - scroll->dcount;
	if (nt < 0)
		nt = 0;
	scroll->top_name = nt - (nt%scroll->cels_per_row);
	draw_scroll_cels(scroll->list_sel);
	rescroll(scroll);
}
void see_scrollbar(Button *b)
{
Name_scroller *scroller = b->group;

	mc_block(b,MC_GREY);
	if (scroller->name_count > 0)
	{
		pj_set_rect(b->root,mc_white(b),b->x+1,b->y+scroller->yoff,b->width-2,
				   scroller->knob_height);
	}
}
void feel_scrollbar(Button *b,Boolean realtime)
/* scrolls list window in response to user mouse on scrollbar sel if realtime
 * is TRUE.  If realtime is false will wait till mouse button is up before
 * redraw */
{
register Name_scroller *scroller;
SHORT spare_height;
SHORT first_top;
SHORT firsty;
SHORT new_top;

	scroller = b->group;
	if(scroller->name_count <= scroller->dcount)
		return;
	if (icb.my > b->y + scroller->yoff + scroller->knob_height)
		repeat_on_pdn(bpagedown,scroller);
	else if (icb.my < b->y + scroller->yoff)
		repeat_on_pdn(bpageup,scroller);
	else
	{
		first_top = scroller->top_name;
		spare_height = b->height - 2 - scroller->knob_height;
		firsty = icb.my;
		for (;;)
		{
			if (!ISDOWN(MBPEN))
				break;
			if (JSTHIT(MMOVE))
			{
				new_top = first_top +
					((icb.my-firsty)*scroller->name_count+(spare_height>>1)) /
					spare_height;

				new_top = clip_scroll(scroller,new_top);
				new_top -= (new_top%scroller->cels_per_row);

				if(new_top != scroller->top_name)
				{
					if(realtime)
					{
						if((new_top -= scroller->top_name) < 0) 
							bscroll_up(b, -new_top); /* new top >= old top */
						else
							bscroll_down(b, new_top); /* new top < old top */
					}
					else
					{
						scroller->top_name = new_top;
						rescroll(scroller);
					}
				}
			}
			wait_any_input();
		}
		if(!realtime)
		{
			new_top = scroller->top_name;
			scroller->top_name = first_top;
			if((new_top -= first_top) < 0)
				bscroll_up(b, -new_top); /* new top >= old top */
			else
				bscroll_down(b, new_top); /* new top < old top */
		}
	}
}
void rt_feel_scrollbar(Button *b)
{
	feel_scrollbar(b,TRUE);
}
void slow_feel_scrollbar(Button *b)
{
	feel_scrollbar(b,FALSE);
}

void redraw_scroller(Name_scroller *scr)
{
	draw_buttontop(scr->list_sel);
	draw_buttontop(scr->scroll_sel);
}

/******* stuff for name scroller where entries are strings ******/

static void xor_1_name(Button *b, void *rast, int x, int y, Names *name, 
					   Boolean hilite)
{
Name_scroller *scroll = (Name_scroller *)(b->group);
(void)name;
(void)hilite;

	pj_xor_rect(rast,mc_white(b)^mc_black(b),x,y,
			 scroll->cel_width,scroll->cel_height);
}
int scroll_name_xoff(Vfont *f)
{
	return(fchar_spacing(f, " ")/2);
}
static void draw_1_name(Button *b, void *rast, int x, int y, Names *name)
{
Name_scroller *scr = (Name_scroller *)(b->group);

	pj_set_rect(rast,mc_black(b),x,y,scr->cel_width,scr->cel_height);
	if(name == NULL)
		return;
	x += scroll_name_xoff(scr->font);
	gftext(rast,scr->font,name->name,x,y+1,mc_white(b),TM_MASK1 );
}
int scroll_names_ysize(Vfont *f, int lines)
{
	return(lines*font_cel_height(f)+2);
}
void init_name_scroller(Name_scroller *scr, Wscreen *screen)

/* initializes a scroller to use the draw_1_name() function, formats it from
 * the font in scroll->font sets the draw_1_sel to draw_1_name 
 * and sets the highlite function to high_1_name() (this xors the entry)
 *
 * The fields that must be loaded in the scroller are:
 * 		scroll_sel 		(scrollbar button)
 * 		list_sel 		(scrolling list button)
 * 		names 			(the list to show)
 *
 * Optional items are:
 *		feel_1_cel() 	(Feelme function called by feel_scroll_cels().)
 *	    font			(If NULL it will default to the screen's font.)
 *		cels_per_row	(If a grid of names is wanted.)
 *
 * Other fields should be NULL.
 */
{
SHORT font_h;

	if(scr->font == NULL)
		scr->font = screen->mufont;
	scale_button(scr->list_sel,&screen->menu_scale);
	scr->draw_1_cel = draw_1_name;
	scr->high_1_cel = xor_1_name;

	scr->border = 1;
	scr->col_xoset = 0;
	scr->row_yoset = 1;
	font_h = font_cel_height(scr->font);
	scr->cels_per_col = (scr->list_sel->height-1)/font_h;

	/* note this kludge: font descenders will cause the cels to hang below
	 * we don't mid chopping off a little bit of the bottom so we do init 
	 * and then compensate height afterwords in case it was truncated by the
	 * divide in the init */

	init_scroller(scr,screen);
	scr->cel_height = font_h;
}
void see_scroll_names(Button *b)
{
	mc_frame(b,MC_GREY);
	mb_isquare(b,mc_black(b));
	draw_scroll_cels(b);
}
