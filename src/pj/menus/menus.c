#include <string.h>
#include <ctype.h>
#include "errcodes.h"
#include "menus.h"
#include "input.h"
#include "memory.h"
#include "ptrmacro.h"
#include "marqi.h"

static void init_group(Wscreen *ws, Mugroup *mg);
static void addto_group(Mugroup *mg, Menuhdr *mh, Menuhdr *over);
static void remfrom_group(Menuhdr *mh);
static void draw_menuwndo(Menuwndo *mw);

Errcode init_muscreen(Wscreen *s)

/* this is called on a newly opened Wscreen to initialize the menu system 
 * control part */
{
	init_group(s,&(s->group0)); /* note this group is not on the stack! */
	init_list(&(s->gstack));
	return(0);
}
void cleanup_muscreen(Wscreen *s)

/* this is called on a used and open Wscreen to cleanup the menu system 
 * control part and close down all the open menus */
{
	if(s == NULL)
		return;
	close_all_menus(s,0);
	s->group0.screen = NULL;
}
void close_all_menus(Wscreen *s,LONG code)
{
Mugroup *mg;

	if(s == NULL
		|| NULL == s->group0.screen) /* not initialized */
	{
		return;
	}

	add_tail(&(s->gstack),&(s->group0.snode));
	while(NULL != (mg = (Mugroup *)get_head(&(s->gstack))))
		close_group_code(mg,code);
	return;
}
static void set_button_roots(Button *mbs, Menuhdr *hdr)

/* sets state of button root and button flags to reflect whether window is
 * open or not and whether the button is attached or not */
{
	while(mbs)
	{
		if(mbs->children != NULL)
			set_button_roots(mbs->children,hdr);

		if(hdr == NULL)
		{
			mbs->root = NULL;
			mbs->flags &= ~(MB_ROOTISWNDO);
		}
		else if(hdr->mw)
		{
			mbs->root = hdr->mw;
			mbs->flags |= MB_ROOTISWNDO;
		}
		else
		{
			mbs->root = (Menuwndo *)hdr;
			mbs->flags &= ~(MB_ROOTISWNDO);
		}
		mbs = mbs->next;
	}
}
Menuhdr *get_button_hdr(Button *b)

/* returns header button is attached to if present using root and flags */
{
	if(b == NULL)
		return(NULL);
	if(b->flags & MB_ROOTISWNDO)
		return(b->root->hdr);
	return((Menuhdr *)(b->root));
}
Menuwndo *get_button_wndo(Button *b)

/* returns menuwndo button is directed to if present using root and flags */
{
	if(b == NULL)
		return(NULL);
	if(b->flags & MB_ROOTISWNDO)
		return(b->root);
	return(((Menuhdr *)(b->root))->mw);
}
static int wndo_domenu(void *w)
/* function called from window io loop in do_xxxloop */
{
	Menuhdr *mh = ((Menuwndo *)w)->hdr;

	if(mh->domenu != NULL)
		return((*(mh->domenu))(mh));
	do_menubuttons(mh);
	return(1);
}
static int wndo_dopull(void *w)
/* function called from window io loop in do_xxxloop */
{
	Menuhdr *mh = ((Menuwndo *)w)->hdr;

	if(mh->domenu != NULL)
		return((*(mh->domenu))(mh));
	return(0);
}

#define SCL(c,p,q) ((long)(c)*(long)(p)/(q))
void scale_rect(Rscale *scale, Rectangle *in, Rectangle *out)
{
	out->x = SCL(in->x, scale->xscalep, scale->xscaleq);
	out->width = SCL(in->width, scale->xscalep, scale->xscaleq);
	out->y = SCL(in->y, scale->yscalep, scale->yscaleq);
	out->height = SCL(in->height, scale->yscalep, scale->yscaleq);
}
void scale_xlist(Rscale *scale, SHORT *in, SHORT *out, int dim)
{
	while(--dim >= 0)
	{
		*out++ = SCL(*in, scale->xscalep, scale->xscaleq);
		++in;
	}
}
void scale_ylist(Rscale *scale, SHORT *in, SHORT *out, int dim)
{
	while(--dim >= 0)
	{
		*out++ = SCL(*in, scale->yscalep, scale->yscaleq);
		++in;
	}
}
void scale_xylist(Rscale *scale, Short_xy *in, Short_xy *out, int dim)
{
	while(--dim >= 0)
	{
		out->x = SCL(in->x, scale->xscalep, scale->xscaleq);
		out->y = SCL(in->y, scale->yscalep, scale->yscaleq);
		++out;
		++in;
	}
}
#undef SCL

void scale_button(Button *b, Rscale *scale)
{
Rectangle rect;

	if(b->flags & MB_NORESCALE)
		return;
	if (!(b->flags & (MB_SCALE_ABSW|MB_SCALE_ABSH)))
		scale_rect(scale,&b->orig_rect,(Rectangle *)(&b->RECTSTART));
	else
	{
		rect = b->orig_rect;
		if(b->flags & MB_SCALE_ABSW)
			rect.width += rect.x - 1;
		if(b->flags & MB_SCALE_ABSH)
			rect.height += rect.y - 1;
		scale_rect(scale,&rect,(Rectangle *)(&b->RECTSTART));
		if(b->flags & MB_SCALE_ABSW)
			b->width = b->width - b->x + 1;
		if(b->flags & MB_SCALE_ABSH)
			b->height = b->height - b->y + 1;
	}
}
static void close_menuwndo(Menuhdr *mh)

/* closes a menus visible window and updates header for state in window */
{
Menuwndo *mw;

	if((mw = mh->mw) == NULL)
		return;
	if(mh->on_showhide)
		(*(mh->on_showhide))(mh,FALSE);
	copy_rectfields(&(mw->w),mh);
	close_wndo((Wndo *)(mw));
	mh->mw = NULL;
	if(mh->type == PANELMENU)
		set_button_roots(mh->mbs,mh);
}
static Errcode open_menuwndo(Wscreen *screen, Menuhdr *mh,Wndo *over)

/* opens a menu window for a menu header reflecting the specs in the 
 * mneu header */
{
Menuwndo *mw;
Errcode err;

	{
	WndoInit wi;

		clear_mem(&wi, sizeof(wi)); /* start cleared */
		copy_rectfields(mh,&wi);  /* load size and position */
		wi.screen = screen;   
		wi.extrabuf = sizeof(Menuwndo) - sizeof(Wndo); /* alloc a Menuwndo */
		wi.flags |= (WNDO_NOCLEAR|WNDO_MUCOLORS);
		wi.over = over;
		if(mh->cursor == NULL)
			wi.cursor = screen->menu_cursor;
		else
			wi.cursor = mh->cursor;
		if((err = open_wndo((Wndo **)&(mh->mw),&wi)) < 0)
			goto error;
		mh->mw->w.procmouse = mh->procmouse;
		mh->mw->w.redraw = (wndo_redraw_func)draw_menuwndo;
	}

	mw = mh->mw;
	mw->hdr = mh; /* set header back pointer */

	mw->w.userid = mh->wndoid;

	if(mh->font == NULL)  /* SCREEN_FONT */
		mw->font = screen->mufont;
	else
		mw->font = mh->font;

	if(!(mh->ioflags) && mh->domenu == NULL)
		mw->w.ioflags = DOBUTTON_INPUT; 
	else
		mw->w.ioflags = mh->ioflags;

	if(mh->type == PANELMENU)
	{
		mw->w.doit = wndo_domenu; 
		set_button_roots(mh->mbs,mh);
	}
	else if(mh->type == PULLMENU)
	{
		mw->w.doit = wndo_dopull; 
	}

	if(mh->on_showhide)
		(*(mh->on_showhide))(mh,TRUE);
	return(0);
error:
	close_menuwndo(mh);
	return(err);
}
void close_menu_code(Menuhdr *mh,LONG code)
{
	if(mh == NULL)
		return;
	close_menuwndo(mh);
	if(mh->group)
		mh->group->retcode = code;
	if(mh->flags & DOMENU_DEFAULT)
		mh->domenu = NULL;
	remfrom_group(mh);
	if(mh->cleanup)
		(*(mh->cleanup))(mh);
}
void close_menu(Menuhdr *mh)
{
	close_menu_code(mh,0);
}
void scale_button_list(Button *b, Rscale *scale)
{
	while(b)
	{
		scale_button(b,scale);
		if (b->children != NULL)
			scale_button_list(b->children,scale);
		b = b->next;
	}
}
static void scale_menu_size(Menuhdr *mh,Rscale *scale)
{
	if(mh->flags & MENU_NORESCALE)
		return;
	mh->width = rscale_x(scale,(mh->orig_rect.width - 1)) + 1;
	mh->height = rscale_y(scale,(mh->orig_rect.height - 1)) + 1;
}

static Boolean
is_rect_and_menu_rect_same(const Rectangle *r, const Menuhdr *m)
{
	/* TODO: Menuhdr.RECT_FIELDS should be a Rectangle.  Don't assume
	 * the spacing between x/y/w/h will be the same due to padding.
	 */
	return (r->x == m->x) && (r->y == m->y)
		&& (r->width == m->width) && (r->height == m->height);
}

Errcode open_menu(Wscreen *screen, Menuhdr *mh,Mugroup *group, Wndo *over)

/* dont open a menu thats already open if group is null open menu over 
 * over if group is non null open on top */
{
Errcode err;
Menuhdr *overmenu;
Wndo *overwndo;

	mh->flags &= ~(DOMENU_DEFAULT);

	scale_menu_size(mh,&screen->menu_scale);

	/* clip if requested and make sure its on screen !! */

	if(mh->flags & MENU_NOBCLIP) /* make sure at least 5 pixels on screen */
	{
		if(mh->x >= screen->wndo.width - 5)
			mh->x = screen->wndo.width - 5;
		else
		{
			if((mh->x += mh->width) < 5)
				mh->x = 5;
			mh->x -= mh->width;
		}
		if(mh->y >= screen->wndo.height - 5)
			mh->y = screen->wndo.height - 5;
		else
		{
			if((mh->y += mh->height) < 5)
				mh->y = 5;
			mh->y -= mh->height;
		}
	}
	else
	{
		bclip_rect((Rectangle *)&(mh->RECTSTART), 
				   (Rectangle *)&(screen->wndo.RECTSTART));
	}

	if(mh->type == PANELMENU && !(mh->flags & MENU_NOMB_RESCALE))
		scale_button_list(mh->mbs, &screen->menu_scale);

	if(group == NULL) /* add to default group */
	{
		overmenu = NULL;
		overwndo = over;
		group = &(screen->group0);
	}
	else
	{
		/* for now always on top if opened in a group */
		overmenu = NULL;
		overwndo = NULL;
	}

	if (!overwndo && !is_rect_and_menu_rect_same(&screen->last_req_pos, mh))
	{
		cancel_reqpos(screen);
	}
	if((err = open_menuwndo(screen,mh,overwndo)) < 0)
		goto error;


	if(mh->domenu == NULL)
	{
		mh->domenu = do_menubuttons;
		mh->flags |= DOMENU_DEFAULT;
	}
	addto_group(group,mh,overmenu);


	redraw_wndo(&(mh->mw->w));
	return(0);
error:
	close_menu(mh);
	return(err);
}
/***********************************************************************/
void hide_menu(Menuhdr *m)

/******  Menu hiding and showing calls:
 * 	o - A hidden menu is open but has its menuwndo closed.
 *  o - A hidden menu has the "roots" in the buttons pointing to the header
 *		instead of the menuwindow which is the case if the window is open.
 *  o - Unhiding (showing) a closed menu will have no effect,
 *  o - Unhiding (showing) a open and visible menu (not hidden) will leave
 *		things unchanged.
 **************************/
{
	if((m == NULL) || (m->mw == NULL))
		return;
	close_menuwndo(m);
	if(m->group == NULL)
		return;
	--(m->group->non_hidden);
}
Errcode show_menu(Menuhdr *m)

/* note: this will not show a menu if it is not open */
{
Errcode err;
Mugroup *mg;
Wndo *over;

	if((m == NULL)
		|| (m->mw != NULL)  /* already hidden */
		|| (NULL == (mg = m->group))) /* no group ?? */
	{
		return(0);
	}

	if(mg->num_menus < 2 || is_tail(&(m->node))) /* open in same position */
		over = NULL;
	else
		over = (Wndo *)(TOSTRUCT(Menuhdr,node,m->node.next)->mw);

	if((err = open_menuwndo(mg->screen,m,over)) < 0)
		goto error;

	draw_menu(m);
	++(mg->non_hidden);
	return(0);
error:
	return(err);
}
/****************** menu group calls ********************/

static void init_group(Wscreen *ws,Mugroup *mg)
{
	clear_mem(mg,sizeof(*mg));
	init_list(&mg->menuhdrs);
	mg->screen = ws;
}
void push_group(Wscreen *ws,Mugroup *mg)
/* opens a mugroup. Puts it on head of stack and Prepares it for menus */
{
	init_group(ws,mg);
	add_head(&(ws->gstack),&(mg->snode));
	++ws->glevel;
}
void pop_group(Mugroup *mg)
{
	safe_rem_node(&(mg->snode));
	if(mg->screen != NULL)
		--mg->screen->glevel;
}
void close_group_code(Mugroup *mg,LONG retcode)

/* removes a group from the stack it is attached to this will close
 * all menus remaining in the group */
{
Menuhdr *m;

	if(mg == NULL)
		return;

	/* close menu will remove menu from group list */

	while(NULL != (m = (Menuhdr *)see_head(&(mg->menuhdrs))))
	{
		m = TOSTRUCT(Menuhdr,node,m);
		close_menu(m);
	}
	mg->retcode = retcode;
}
void close_group(Mugroup *mg)
{
	close_group_code(mg,0);
}
void mh_gclose_code(Menuhdr *mh, LONG code)
/* close menuheaders group */
{
	if(mh == NULL)
		return;
	close_group_code(mh->group,code);
}
static void addto_group(Mugroup *mg,Menuhdr *mh,Menuhdr *over)
{
	if(mh->mw->w.W_screen != mg->screen) /* oops! window on wrong screen */
		return;

	if(over == NULL)
		add_head(&mg->menuhdrs,&(mh->node));
	else
		insert_before(&(mh->node),&(over->node));

	mh->group = mg;
	if(mh->mw != NULL)
		++mg->non_hidden;
	++mg->num_menus;
}
static void remfrom_group(Menuhdr *mh)
{
Mugroup *mg;

	if(NULL == (mg = mh->group))
		return;
	safe_rem_node(&(mh->node));
	if(mh->mw != NULL)
		--mg->non_hidden;
	--mg->num_menus;
	mh->group = NULL;
}
void hide_group(Mugroup *mg)
/* hides all visible menus in a group */
{
Dlnode *next;
Menuhdr *m;

	if((mg == NULL)
	   || !(mg->non_hidden))
	{
		return;
	}

	for(m = (Menuhdr *)(mg->menuhdrs.head);
		NULL != (next = ((Dlnode *)m)->next);
		m = (Menuhdr *)next)
	{
		m = TOSTRUCT(Menuhdr,node,m);
		if(m->mw == NULL)
			continue;
		close_menuwndo(m);
	}
	mg->non_hidden = 0;
	return;
}
Errcode show_group(Mugroup *mg)

/* shows all menus that are currently hidden and not closed in order and
 * positions currently in the list starts with backmost and draws up */
{
Dlnode *prev;
Menuhdr *m;
Errcode err;

	if( (mg == NULL)
		|| (mg->screen == NULL))
	{
		return(Err_bad_input);
	}

	if(mg->non_hidden == mg->num_menus)
		return(0);

	for(m = (Menuhdr *)(mg->menuhdrs.tails_prev);
		NULL != (prev = ((Dlnode *)m)->prev);
		m = (Menuhdr *)prev)
	{
		m = TOSTRUCT(Menuhdr,node,m);
		if(m->mw == NULL)
		{
			if((err = open_menuwndo(mg->screen,m,NULL)) < 0)
				goto error;
			++mg->non_hidden;
			draw_menu(m);
		}
	}
	mg->non_hidden = mg->num_menus;
	return(0);
error:
	return(err);
}

Boolean cgroup_hidden(Wscreen *ws)
{
Mugroup *mg;

	if(NULL == (mg = (Mugroup *)see_head(&ws->gstack)))
		return(TRUE);
	return(mg->hmpcnt > 0);
}
void stack_hide_cgroup(Wscreen *ws)

/* these use a stack oriented protocall and will only hide-show if stack 
 * is at a cusp allowing recursive bracketing of things with hide-show calls.
 * note that if anything is showing it will re-hide it !!! */
{
Mugroup *mg;
	
	if(NULL != (mg = (Mugroup *)see_head(&ws->gstack)))
	{
		++mg->hmpcnt;
		hide_group(mg);
	}
}
Boolean stack_show_cgroup(Wscreen *ws)

/* this will only show things when stack is <= 1 */
{
Mugroup *mg;

	if(NULL != (mg = (Mugroup *)see_head(&ws->gstack)))
	{
		if((--mg->hmpcnt) <= 0)
		{
			show_group(mg);
			mg->hmpcnt = 0; /* in case someone blew it */
			return(1);
		}
	}
	return(0);
}
/*************************************************************************/
/* menu moving */

void offset_button_list(Button *b, SHORT x, SHORT y)
/* This adds an offset to a tree of buttons */
{
if (b == NULL)
	return;
offset_button_list(b->next,x,y);
offset_button_list(b->children,x,y);
b->x += x;
b->y += y;
}

static Boolean marqmove_menu(Menuhdr *m, int clipit)
{
Menuwndo *mw;
Rectangle *clip;

	if(m == NULL || (mw = m->mw) == NULL)
		return(0);

	if(clipit)
		clip = (Rectangle *)&(mw->w.W_screen->wndo.RECTSTART);
	else
		clip = NULL;

	if(!marqmove_wndo(&mw->w,clip))
		return(0);

	copy_rectfields(&(mw->w),m);
	return(1);
}
static Boolean menu_to_bottom(Menuhdr *m)
{
Rectangle *clip;
Menuwndo *mw;
Rectangle newpos;



	if(m == NULL || (mw = m->mw) == NULL)
		return(0);
	clip = (Rectangle *)&(mw->w.W_screen->wndo.RECTSTART);
	newpos.width = m->width;
	newpos.height = m->height;
	newpos.x = 0;
	newpos.y = clip->height - newpos.height;
	if (!reposit_wndo(&mw->w,&newpos,NULL))
		return(0);
	copy_rectfields(&(mw->w),m);
	return(1);
}
void mb_menu_to_bottom(Button *b)
{
	menu_to_bottom(get_button_hdr(b));
}
void mb_move_menu(Button *b)
{
	marqmove_menu(get_button_hdr(b),0);
}
void mb_clipmove_menu(Button *b, void *dat)
{
	(void)dat;
	marqmove_menu(get_button_hdr(b),1);
}
/*************************************************************************/
void draw_buttontop(Button *b)

/******* calls to do button and menu drawing *****************************/
/* Draws top level button only, not its children and not any siblings
 * this may be called whether or not a button is attached to an open menu */
{
	if (b != NULL 
		&& (b->flags & MB_ROOTISWNDO)
		&& (b->seeme != NULL))
	{
		(*b->seeme)(b);
	}
}
void draw_buttonlist(register Button *b)

/* draws all buttons and children starting with first one given 
 * this may be called if a button is NOT attached to an open menu 
 * safely  and it will not draw */
{
	if((b == NULL)
		|| !(b->flags & MB_ROOTISWNDO))
	{
		return;
	}

	for(;;)
	{
		if (b->seeme != NULL)
			(*b->seeme)(b);
		if (b->children != NULL)
			draw_buttonlist(b->children);
		if((b = b->next) == NULL)
			break;
	}
}
void draw_button(Button *b)

/* draws a button and its children, children first
 * this may be called whether or not a button is attached to an open menu 
 * or button is NULL */
{
	if( (b != NULL) 
		&& (b->flags & MB_ROOTISWNDO))
	{
		if(b->seeme != NULL)
			(*b->seeme)(b);
		if(b->children != NULL)
			draw_buttonlist(b->children);
	}
}
void draw_menu(Menuhdr *mh)
{
	if(mh && mh->mw)
		redraw_wndo(&(mh->mw->w));
}
static void draw_menuwndo(Menuwndo *mw)
{
Menuhdr *mh;

	if(!mw)
		return;

	mh = mw->hdr;

	if(mh->seebg != NULL)
		(*(mh->seebg))(mw);
	else
		seebg_white(mw);

	switch(mh->type)
	{
		case PANELMENU: 
			draw_buttonlist(mh->mbs); 
			break;
		case PULLMENU: 
			draw_menupull(mh);
			break;
	}
}
/*************************************************************************/
/************ functions to process input to menus and buttons ************/

Boolean cursin_menu(Menuhdr *m)
/* returns 1 if screen crursor is in the menu's window 0 if window is not open
 * (menu hidden or closed) or cursor is not in the window */
{
	if(m->mw == NULL)
		return(0);
	return(ptin_wndo(&(m->mw->w),icb.sx,icb.sy));
}

typedef struct bhitdat {
	Button *one_hit;
	SHORT x;
	SHORT y;
} Bhitdat;

static Boolean check_hit(Button *b, Bhitdat *bhd)

/* recursive sub for hit_button we've got the stack for it */
{
	if(b)
	{
		if(check_hit(b->next,bhd))
			return(1);
		if(check_hit(b->children,bhd))
			return(1);
		if(ptin_button(b,bhd->x,bhd->y))
		{
			bhd->one_hit = b;
			return(1);
		}
	}
	return(0);
}

Button *hit_button(Button *b,SHORT x,SHORT y)

/* returns button hit in button list for x and y if one was */
{
Bhitdat bhd;

	bhd.x = x;
	bhd.y = y;
	bhd.one_hit = NULL;
	check_hit(b,&bhd);
	return(bhd.one_hit);
}
static Button *key_butn(Button *b,SHORT key)

/* sub for key_button() **/
{
Button *gotone;

	while(b != NULL)
	{
		if(b->children != NULL)
		{
			if((gotone = key_butn(b->children,key)) != NULL)
				return(gotone);
		}
		if(key == b->key_equiv)
			return(b);
		b = b->next;
	}
	return(NULL);
}
static Button *key_button(Button *first, SHORT iokey)
/* returns button in list with key equivalent equal to key */
{
unsigned char c;

	if(first == NULL)
		return(NULL);

	c = iokey;
	if(c != 0)
		iokey = tolower(c);
	return(key_butn(first,iokey));
}

static void check_reqpos_cancel(Menuhdr *mh)
{
Menuwndo *mw = mh->mw;

	if(	mw != NULL &&
		!is_rect_and_menu_rect_same(&(mw->w.W_screen->last_req_pos), mh))
	{
		cancel_reqpos(mw->w.W_screen);
	}
}
static void menu_set_tabnext(Menuhdr *mh, Button *b)
{
	if(mh && mh->group)
		mh->group->tabnext = b;
}
void mb_set_tabnext(Button *b, Button *next)
{
	menu_set_tabnext(get_button_hdr(b),next);
}
int button_keyhit(Menuhdr *mh,Button *mbs,VFUNC prehit)

/* processes a mouse or key equivalent hit for a button list 
 * returns 1 if we got a mouse hit 2 if a key hit 0 if no hit
 * if not calls feelme or optme for hit 
 * calls prehit function before calling feelme or optme if hit */
{
Button *b;
int ret;
VFUNC doit;
Menuwndo *mw;
SHORT bflags;

	if(JSTHIT(KEYHIT)) /* keys take priority */
	{
		if((UBYTE)icb.inkey == '\t' 
			&& (b = mh->group->tabnext) != NULL
			&& b->root == mh->mw )
		{
			goto got_keyhit;
		}
		else if(NULL != (b = key_button(mbs,icb.inkey)))
		{
got_keyhit:
			if(!(b->flags & MB_DISABLED) && ((doit = b->feelme) != NULL))
			{
				bflags = 0;
				ret = 2; /* key hit */
				goto do_hit;
			}
		}
	}
	else if(JSTHIT(MBPEN|MBRIGHT))
	{
		mw = mh->mw;
		if(NULL != (b = hit_button(mbs,icb.cx - mw->w.x,
									   icb.cy - mw->w.y)))
		{
			bflags = b->flags;
			if(JSTHIT(MBPEN))
			{
				if(!(bflags & MB_DISABLED) && ((doit = b->feelme) != NULL))
				{
					check_reqpos_cancel(mh);
					ret = 1; /* mouse hit */
					goto do_hit;
				}
			}
			else
			{
				if( ((bflags & (MB_DISABLED|MB_DISABOPT)) != 
							          (MB_DISABLED|MB_DISABOPT))
					&& !(bflags & MB_OPTISDATA)
					&& ((doit = b->optme) != NULL))
				{
					check_reqpos_cancel(mh);
					ret = 1; /* mouse hit */
					goto do_hit;
				}
			}
		}
	}
	return(0); /* no hit */

do_hit:

	mh->group->tabnext = NULL;
	if((bflags & (MB_HIONSEL|MB_HILITE_TYPES)) == MB_HIONSEL)
	{
		b->flags ^= MB_HILIT; 
		(*(b->seeme))(b);
	}

	if(prehit != NULL)
		(*prehit)(b);
	(*doit)(b);

	if((bflags & (MB_HIONSEL|MB_HILITE_TYPES)) == MB_HIONSEL) 
	{
		b->flags ^= MB_HILIT;
		wait_mbup(MBPEN|MBRIGHT);
		if(MENU_ISUP(mh))
			(*(b->seeme))(b);
	}
	return(ret);
}
Boolean is_abortkey(void)
{
	return(((UBYTE)icb.inkey) == ' ' || ((UBYTE)icb.inkey) == ESCKEY);
}
int check_reqabort(Menuhdr *hdr)

/* returns 1 if ate key */
{
Mugroup *mg;

	mg = hdr->group;
	if((mg->flags & MUG_REQUESTOR)
		&& (JSTHIT(KEYHIT) && is_abortkey())) 
	{
		close_group_code(mg,Err_abort);
		return(2); /* ate key */
	}
	return(0);
}
int do_menubuttons(Menuhdr *hdr)
/* default group menu processor returns 1 if ate mouse 2 if ate key input */
{
int ret;

	if(0 != (ret = button_keyhit(hdr,hdr->mbs,NULL)))
		return(ret);
	return(check_reqabort(hdr));
}
void menu_to_point(Wscreen *s,Menuhdr *mh,SHORT centx,SHORT centy)

/* sets x and y of unopened or hidden menu to center it at current about a 
 * position */ 
{
	if(mh->mw != NULL) /* not if open */
		return;

	scale_menu_size(mh,&s->menu_scale);

	mh->x = centx - mh->width/2; /* center it about coords given */	
	mh->y = centy - mh->height/2;	

	bclip_rect((Rectangle *)&(mh->RECTSTART),
			   (Rectangle *)&(s->wndo.RECTSTART));
}

void menu_to_cursor(Wscreen *s,Menuhdr *mh)

/* sets x and y of unopened menu to center it at current screen cursor
 * position */ 
{
	menu_to_point(s,mh,icb.sx,icb.sy);
}
void menu_to_reqpos(Wscreen *s,Menuhdr *mh)

/* sets x and y of unopened menu to center it at current screen cursor
 * position modified requestor position */ 
{
	if(mh->mw != NULL) /* not if open */
		return;

	scale_menu_size(mh,&s->menu_scale);

	get_requestor_position(s,mh->width,mh->height,
						   (Rectangle *)&(mh->RECTSTART));	
}

/* enable and disable manipulators */

Boolean set_button_disable(Button *b, Boolean disable)
{
	if(disable)
		b->flags |= MB_DISABLED;
	else
		b->flags &= ~(MB_DISABLED);
	return(disable);
}
void draw_button_disable(Button *b, Boolean disable)
/* combo set disable and redraw */
{
	if(disable)
		b->flags |= MB_DISABLED;
	else
		b->flags &= ~(MB_DISABLED);
	draw_button(b);
}
void enable_button(Button *b)

/* enables and redraws button */
{
	b->flags &= ~(MB_DISABLED);
	draw_buttontop(b);
}
void disable_button(Button *b)

/* disables and redraws button */
{
	b->flags |= MB_DISABLED;
	draw_buttontop(b);
}
void set_mbtab_disables(Button **bt,Boolean disable)
{
Button *b;

	while((b = *bt++) != NULL)
	{
		if(disable)
			b->flags |= MB_DISABLED;
		else
			b->flags &= ~(MB_DISABLED);
	}
}
#ifdef SLUFFED
void enable_mbtab(Button **bt)
/* disables a table of buttons terminated by a null */
{
Button *b;

	while((b = *bt++) != NULL)
		b->flags &= ~(MB_DISABLED);
}
void disable_mbtab(Button **bt)
/* disables a table of buttons terminated by a null */
{
Button *b;

	while((b = *bt++) != NULL)
		b->flags |= MB_DISABLED;
}
#endif /* SLUFFED */


static void free_blist(Button *b)

/* frees a ram copy of a button list */
{
	if(b)
	{
		free_blist(b->children);
		free_blist(b->next);
		pj_free(b);
	}
}
void free_buttonlist(Button **pb)
{
	free_blist(*pb);
	*pb = NULL;
}
Errcode clone_buttonlist(Button *toclone,Button **pb)

/* makes a ram copy of, and links a list of buttons */
{
	if(!toclone)
	{
		*pb = NULL;
		return(Success);
	}

	if(NULL == (*pb = pj_malloc(sizeof(Button))))
	{
		return(Err_no_memory);
	}
	copy_mem(toclone,*pb,sizeof(Button));
	(*pb)->children = NULL;
	if( (clone_buttonlist(toclone->next,&((*pb)->next)) < 0)
		|| (clone_buttonlist(toclone->children,&((*pb)->children)) < 0))
	{
		free_buttonlist(pb);
		return(Err_no_memory);
	}
	return(Success);
}
Button *find_button(Button *m, SHORT id)

/* given root returns first occurence found of button with id value */
{
Button *child;

	while(m)
	{
		if (m->identity == id)
			return(m);
		if(m->children)
		{
			if(NULL != (child = find_button(m->children,id)))
				return(child);
		}
		m = m->next;
	}
	return(NULL);
}
