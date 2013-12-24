#define INPUT_INTERNALS
#include "errcodes.h"
#include "memory.h"
#include "ptrmacro.h"
#include "input.h"
#include "menus.h"
#include "marqi.h"


Boolean menu_in_active_group(Menuhdr *mh)
{
	if(mh->group == NULL)
		return(FALSE);
	return(mh->group ==	(Mugroup *)see_head(&mh->group->screen->gstack));
}

/***************** input waits while scanning windows ***************/

static int anim_check_wndos(Wndo **ciwin)
{
Wndo *w;
Dlnode *next;

	*ciwin = NULL;
	for(w = (Wndo *)(icb.input_screen->wilist.head);
		next = ((Dlnode *)w)->next;
		w = (Wndo *)next )
	{
		w = TOSTRUCT(Wndo,W_node,w);
		if(w->flags & WNDO_HIDDEN)
			continue;
		if(ptin_wndo(w,icb.sx,icb.sy))
		{
			*ciwin = w;
			break;
		}
	}
	load_wndo_iostate(*ciwin);
	return(0);
}

static int anim_check_menus(Wndo **ciwin)
{
Menuhdr *mh;
Dlnode *next;

	*ciwin = NULL;
	for(mh = (Menuhdr *)(icb.input_screen->gstack.head);
	    NULL != (next = ((Dlnode *)mh)->next);
		mh = (Menuhdr *)next )
	{
		mh = TOSTRUCT(Menuhdr,node,mh);
		if(cursin_menu(mh))
		{
			*ciwin = &(mh->mw->w);
			break;
		}
	}
	load_wndo_iostate(*ciwin);
	return(0);
}

Wndo *set_input_wndo()

/* sets input window to window under cursor */
{
Wndo *iowndo = NULL;

	anim_check_wndos(&iowndo);
	return(iowndo);
}

Wndo *wait_wndo_input(ULONG ioflags)

/* this is like wait input but loads cursors and io state as it passes 
 * windows returns pointer to window under cursor NULL if over screen 
 * leaves iostate set to whatever the last hit window was */
{
Wndo *iowndo = NULL;

	display_cursor();
	anim_wait_input(ioflags,ANY_INPUT,-1,anim_check_wndos,&iowndo);
	undisplay_cursor();
	return(iowndo);
}

typedef struct win_anim_data {
	Wndo *iowndo;
	ULONG forceflags;
	FUNC afunc;
	void *adata;
} Wadat;

static int anim_anim(Wadat *wd)
{
	anim_check_wndos(&(wd->iowndo));
	if(JSTHIT(wd->forceflags|ICB_TIMEOUT))
		return((*(wd->afunc))(wd->adata));
	return(0);
}

int anim_wndo_input(ULONG waitflags, ULONG forceflags, 
				    int maxfields, FUNC func, void *funcdata)

/* same as anim_wait_input() but scans windows and switches iostate and 
 * cursors.  does not save or restore io environment so current 
 * icb.iowndo will be set to current cursor window mouse is in */
{
Wadat wd;

	wd.iowndo = NULL;
	wd.forceflags = forceflags;
	wd.afunc = func;
	wd.adata = funcdata;
	return(anim_wait_input(waitflags,forceflags|MMOVE|ICB_TIMEOUT,
								   maxfields,anim_anim,&wd));

}

Wndo *wait_menu_input(ULONG ioflags)

/* this is like wait wndo input but only looks at current group if it is
 * a requestor group */
{
Mugroup *mg;
FUNC afunc;
Wndo *iowndo = NULL;

	display_cursor();
	mg = (Mugroup *)see_head(&(icb.input_screen->gstack));
	if(mg != NULL && mg->flags & MUG_REQUESTOR)
		afunc = anim_check_menus;
	else
		afunc = anim_check_wndos;

	anim_wait_input(ioflags,ANY_INPUT,-1,afunc,&iowndo);
	undisplay_cursor();
	return(iowndo);
}
/***************** menu opener processor loops ******************/

static Errcode startloop_open(Mugroup *mg, 
							  Menuhdr *menu,
							  Button *initb, Menuhdr *pull)
{
Errcode err;

	if(pull != NULL)
	{
		if((err = open_menu(mg->screen, pull, mg, NULL)) < 0)
			goto error;
	}
	if(menu != NULL)
	{
		if((err = open_menu(mg->screen, menu, mg, NULL)) < 0)
			goto error;

		if (initb != NULL && initb->feelme != NULL)
		{
			load_wndo_mouset(&(menu->mw->w));
			(*initb->feelme)(initb);
		}
	}
	load_wndo_iostate(NULL); /* start off with screen default */

	return(0);
error:
	return(err);
}

static int do_closegroup(Mugroup *mg)
{
	close_group_code(mg,Err_abort);
	return(1);
}
static int do_doitfunc(Wscreen *s, FUNC func, void *dat)
{
int ret;

	disable_wrefresh(s);
	if(icb.mcurs_up > 0)
		undisplay_cursor();
	ret = (*(func))(dat);
	enable_wrefresh(s);
	return(ret);
}
#define do_wndodoit(w) do_doitfunc(screen,(w)->doit,w)

static void find_wndo_cursor(Wscreen *screen)
{
register Dlnode *next;
register Wndo *w;

	for(w = (Wndo *)(screen->wilist.head);
		next = ((Dlnode *)w)->next;
		w = (Wndo *)next )
	{
		w = TOSTRUCT(Wndo,W_node,w);
		if(w->flags & WNDO_HIDDEN)
			continue;
		if(ptin_wndo(w,icb.sx,icb.sy))
		{
			load_wndo_iostate(w);
			break;
		}
	}
}

LONG do_menuloop(Wscreen *screen,register Menuhdr *mh,Button *initb,
				 Menuhdr *pull, FUNC default_doclick )

/* the big loop that is given a pointer to an unopened menu header
 * to start the loop with. as long as there is at least 1 menu or a pull in the
 * group left it will continue looping. if its the bottom level for the
 * screen it will wait until group0 is empty also */
{
Mouset mset;
Mugroup mg;
register Dlnode *next;
register Wndo *w, *hitwndo;

	if(screen->glevel)
		enable_wrefresh(screen);
	get_mouset(&mset);
	push_group(screen,&mg);

	if((mg.retcode = startloop_open(&mg,mh,initb,pull)) < 0)
		goto error;

	find_wndo_cursor(screen);

	while(mg.num_menus)
	{
		if(icb.mcurs_up <= 0)
			display_cursor();

		mac_wait_input(ANY_INPUT,ANY_INPUT&(~MMOVE));

		hitwndo = NULL; /* no hits yet */

		if(JSTHIT(KEYHIT))
		{
			/* check for hidden menus that want keys */

			for(mh = (Menuhdr *)(mg.menuhdrs.head);
			    NULL != (next = ((Dlnode *)mh)->next);
				mh = (Menuhdr *)next )
			{
				mh = TOSTRUCT(Menuhdr,node,mh);
				if(!(mh->flags & MENU_KEYSONHIDE))
					continue;
				if(mh->mw != NULL || !(mh->ioflags & KEYHIT))
					continue;
				/* ate key! reset mouse wait again */
				if(do_doitfunc(screen,mh->domenu,mh))
					goto reset_mouse;
			}
		}

		for(w = (Wndo *)(screen->wilist.head);
			next = ((Dlnode *)w)->next;
			w = (Wndo *)next )
		{
			w = TOSTRUCT(Wndo,W_node,w);

			if(w->flags & WNDO_HIDDEN)
				continue;

			if(ptin_wndo(w,icb.sx,icb.sy))
			{
				if(hitwndo == NULL)
				{
					hitwndo = w;
					load_wndo_iostate(w);
				}
			}

			if(ISINPUT(KEYHIT)) /* keys take priority */ 
			{
				if(w->doit != NULL && (KEYHIT & w->ioflags))
				{
					if(w != hitwndo) 
						load_wndo_mouset(w);
					if(do_wndodoit(w))
						goto reset_mouse;
				}
			}
			else if(hitwndo != NULL) /* w == hitwndo here */
			{
				if(ISINPUT(w->ioflags)) /* wants and got non key */
				{
					if(w->doit != NULL)
					{
						if(!do_wndodoit(w))
							break;
						goto reset_mouse;
					}
					goto continue_big_loop;
				}
				break;
			}
		}

		if(default_doclick == NULL || !JSTHIT(ANY_CLICK))
			continue;

		if(!do_doitfunc(screen,default_doclick,NULL))
			continue; 

	reset_mouse: /* this is here because a window may be closed etc if input is
				  * processed */

		find_wndo_cursor(screen);

	continue_big_loop:
		continue;
	}
	if(icb.mcurs_up > 0)
		undisplay_cursor();
error:
	pop_group(&mg);
	load_mouset(&mset);
	if(screen->glevel)
		{
		disable_wrefresh(screen);
		}
	return(mg.retcode);
}
static void find_menu_cursor(Mugroup *mg)

/* detects what menu cursor is over and loads it's cursor etc into input */
{
register Dlnode *next;
register Menuhdr *mh;

	for(mh = (Menuhdr *)(mg->menuhdrs.head);
	    NULL != (next = ((Dlnode *)mh)->next);
		mh = (Menuhdr *)next )
	{
		mh = TOSTRUCT(Menuhdr,node,mh);
		if(cursin_menu(mh))
		{
			load_wndo_iostate(&(mh->mw->w));
			return;
		}
	}
	load_wndo_iostate(NULL);
}
LONG do_reqloop(Wscreen *screen,register Menuhdr *mh,Button *initb,
			    Menuhdr *pull, FUNC default_doclick)

/***** call to open put up a requestor menu and input loop on the stack ***
 * will disable all input switching to other windows until it is removed 
 * via mb_close_ok() etc and will always re-show the group if it is all hidden
 * by a feelme or optme it will restore the mouse settings to the ones
 * present on entry it will return the retcode from the last menu close
 * or group close (on the current group) or Err_abort if it was canceled
 * by a right click or space bar */
{
Mouset mset;
Mugroup mg;
Menuwndo *mw;
Menuwndo *hitwndo;
register Dlnode *next;
int altmouset;

	if(screen->glevel)
		enable_wrefresh(screen);
	get_mouset(&mset);
	push_group(screen,&mg);
	mg.flags |= MUG_REQUESTOR;

	if((mg.retcode = startloop_open(&mg,mh,initb,pull)) < 0)
		goto error;

	find_menu_cursor(&mg);

	while(mg.num_menus)
	{
		if(!(mg.non_hidden))
			show_group(&mg);  /* make sure something is visible */

		if(icb.mcurs_up <= 0)
			display_cursor();

		mac_wait_input(ANY_INPUT,ANY_INPUT&(~MMOVE));

		hitwndo = NULL; /* no hits yet */
		altmouset = 0;

		for(mh = (Menuhdr *)(mg.menuhdrs.head);
		    NULL != (next = ((Dlnode *)mh)->next);
			mh = (Menuhdr *)next )
		{
			mh = TOSTRUCT(Menuhdr,node,mh);

			if((mw = mh->mw) != NULL)
			{
				if(hitwndo == NULL) /* first one hit is it */
				{
					if(cursin_menu(mh))
					{
						hitwndo = mw;
						load_wndo_iostate(&(mw->w));
					}
				}
			}
			if(ISINPUT(KEYHIT & mh->ioflags)) /* keys take priority */ 
			{
				if(mw != NULL)
				{
					altmouset = 1;
					load_wndo_mouset(&(mw->w));
					if(do_wndodoit(&mw->w))
						goto reset_mouse;
				}
				else if(mh->flags & MENU_KEYSONHIDE) /* always get keys */
				{
					if(do_doitfunc(screen,mh->domenu,mh))
						goto reset_mouse;
				}
			}
		}

		if(hitwndo != NULL)
		{
			if(ISINPUT(hitwndo->w.ioflags & ~KEYHIT))
			{
				if(!do_wndodoit(&hitwndo->w))
					goto check_default_io;
				goto continue_big_loop;
			}
		}

		/* if we got to here we didn't hit a window or do a key !! 
		 * check for abort ! */

		if((!hitwndo && JSTHIT(MBRIGHT)) || (JSTHIT(KEYHIT) && is_abortkey()))
		{
			do_doitfunc(screen,do_closegroup,&mg);
			break;
		}

	check_default_io:

		if( default_doclick != NULL 
			&& JSTHIT(ANY_CLICK)
			&& do_doitfunc(screen,default_doclick,NULL))
		{
			goto reset_mouse;
		}

		if(altmouset)
			goto reset_mouse;

		if(hitwndo == NULL)
			load_wndo_iostate(NULL);

		goto continue_big_loop;

	reset_mouse:
		find_menu_cursor(&mg);

	continue_big_loop:
		continue;
	}
	if(icb.mcurs_up > 0)
		undisplay_cursor();
error:
	pop_group(&mg);
	load_mouset(&mset);
	if(screen->glevel)
		{
		disable_wrefresh(screen);
		}
	return(mg.retcode);
}
