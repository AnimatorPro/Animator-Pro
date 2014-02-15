#include <ctype.h>
#include <stdio.h>
#include "errcodes.h"
#include "menus.h"
#include "memory.h"
#include "input.h"


void init_pullwork(Pullwork *pw, Menuhdr *mh)
/* initializes a pullwork struct prior to calling pull subroutines 
 * pull menuhdr must be open !! */
{
	clear_mem(pw,sizeof(*pw));
	pw->root = mh;
	pw->screen = mh->group->screen;
	pw->port = pw->screen->viscel;     /* default port */
	if((pw->font = mh->font) == NULL)
		pw->font = pw->screen->mufont;
	pw->spwidth = fchar_spacing(pw->font," ");
	pw->cheight = tallest_char(pw->font);
}

static Bytemap *save_behind(Wscreen *ws,SHORT x,SHORT y,SHORT w,SHORT h)

/* saves a rectangle of the screen for the location and size specified */
{
Bytemap *saverast;
Rasthdr spec;

	copy_rasthdr(ws->viscel,&spec);

	spec.x = x;
	spec.y = y;
	spec.height = h;
	spec.width = w;

	if((pj_alloc_bytemap(&spec,&saverast)) < 0)
		return(NULL);	/* not enough memory */

	disable_wtasks(); /* these pull leaves are not windows so we have to 
					   * disable tasks that may corrupt the screen. These
					   * stack up levels. */
	pj_blitrect(ws->viscel,x,y,saverast,0,0,w,h);
	return(saverast);
}
static void rest_behind(Wscreen *ws, Raster **psaverast)

/* restores a saverast to location it was grabbed from with save_behind 
 * and frees it */
{
Raster *saverast = *psaverast;

	if(saverast != NULL)
	{
		pj_blitrect(saverast,0,0,ws->viscel,saverast->x,saverast->y,
										 saverast->width,saverast->height);
		pj_rast_free(saverast);
		*psaverast = NULL;
		enable_wtasks(); /* re enable tasks disabled when this was saved */
	}
}
static Bytemap *savedraw_pull(int x,int y,Pull *p,Pullwork *pw)

/* draws a pull and allocates a Bytemap and saves underneath pull in it */
{
Bytemap *saverast;

	if((saverast = save_behind(pw->screen, x, y, p->width,p->height))
	!= 	NULL)
		see_pull(x,y,p,pw);
	else
		printf("No pull memory ***************");
	return(saverast);
}

static void unselect(Pullwork *pw)
{
Pull *lp = pw->leaf_parent;

	if(pw->level < 0)
		return;
	if(pw->behind[pw->level] != NULL)
		{
		rest_behind(pw->screen, &(pw->behind[pw->level]));
		if (pw->level == 0)
			{
			lp->flags &= ~PULL_HILIT;
			(*(lp->see))(pw->lpx,pw->lpy,lp,pw);
			}
		}
}

static Pull *which_key_pull(Pull *p,SHORT key)
/* given a list of pulls and an keyboard scan code returns pull in list
 * with matching key, or NULL if none match */
{
SHORT askey;

	/* convert scan-code to straight ascii upper case unless it's
	 * something totally non-ascii like a cursor key. */
	if ((askey = (key&0xff)) != 0)
		{
		key = askey;
		key = toupper(key);
		}
	while(p != NULL)
	{
		if((p->flags & (PULL_DISABLED|PULL_DOESKEYS)) == PULL_DOESKEYS)
			if (toupper(p->key2) == key)
				return(p);
		p = p->next;
	}
	return(NULL);
}

static int do_pullkeys(Menuhdr *mh,SHORT *hitid)
/* called to check a pull for key equivalent hits this could be 
 * recursive to handle more levels if if gets a hit it calls the selit 
 * function  returns 0 if no keys processed Err_not_found if some keys 
 * processed but no hits 1 if we got a hit */
{
int ret;
int was_hidden;
Pull *cchild, *sel;
int ccx, ccy;
Pullwork pw;


	if((cchild = which_key_pull(mh->mbs, icb.inkey)) == NULL)
		return(0); /* did not eat key, no hits */

	init_pullwork(&pw,mh);

	if(mh->mw == NULL)
	{
		was_hidden = 1;
		show_menu(mh);
	}
	else
		was_hidden = 0;

	ccx = mh->x + cchild->x;
	ccy = mh->y + cchild->y;
	pw.leaf_parent = cchild;
	pw.lpx = ccx;
	pw.lpy = ccy;
	cchild->flags |= PULL_HILIT;
	(*cchild->see)(ccx, ccy, cchild, &pw);

	cchild = cchild->children;
	ccx += cchild->x;
	ccy += cchild->y;
	if((pw.behind[pw.level] 
	= 	(Raster *)savedraw_pull(ccx, ccy, cchild, &pw)) == NULL)
		goto nohit;

	hide_mouse();
	wait_click();
	show_mouse();

	if(!JSTHIT(KEYHIT))
		goto nohit;

	if((sel = which_key_pull(cchild->children, icb.inkey)) == NULL)
		goto nohit;

	ret = 1;  /* we got a hit */
	*hitid = sel->id;
	goto cleanup;

nohit:
	ret = Err_not_found;
cleanup:
	unselect(&pw);
	if(was_hidden)
		hide_menu(mh);
	return(ret);
}

static Boolean in_pblock(SHORT x,SHORT y,register Pull*p)
{
	if (   (icb.sx < x) 
		|| (icb.sy < y)
		|| (icb.sx >= (x + p->width))
		|| (icb.sy >= (y + p->height)))
	{
		return(0);
	}
	return(1);
}
static int do_pullmouse(Menuhdr *mh,SHORT *hitid)

/* returns 1 or Err_disabled if we got a hit Err_not_found if no hit */
/* This routine is pretty creaky, but who want's to rewrite it? */
{
SHORT x, y;  /* root offset */
Pullwork pw;
int ret;
int in_cchild;
Pull *child, *cchild;
SHORT cx,cy,ccx,ccy;
int gothit;
Pull *p0,*p1;

	reuse_input();		/* make nice for loop */
	init_pullwork(&pw,mh);
	x = mh->x;
	y = mh->y;
	cchild = NULL;
	p0 = NULL; 
	/* note that here pw.level = 0 */

	for (;;)
	{
		wait_any_input();
		if (JSTHIT(KEYHIT))
		{
			ret = Err_not_found;
			reuse_input();
			goto cleanup;
		}
		if(cursin_menu(mh))		/* cursor in menu bar (not leaf yet) */
		{
			child = mh->mbs;

			for(;;)
			{
				if(child == NULL)
				{
					unselect(&pw);
					p0 = NULL; 
					cchild = NULL;
					goto nochild;
				}

				cx = x + child->x;
				cy = y + child->y;

				if(!in_pblock(cx, cy, child))
				{
					child = child->next;
					continue;
				}

				if(p0 != child)
				{
					unselect(&pw);
					pw.leaf_parent = p0 = child;
					pw.lpx = cx;
					pw.lpy = cy;
					child->flags |= PULL_HILIT;
					(*child->see)(cx, cy, child, &pw);

					if ((cchild = child->children) != NULL)
					{
						ccx = cx + cchild->x;
						ccy = cy + cchild->y;
						if(NULL == (pw.behind[0] 
						= 	(Raster *)savedraw_pull(ccx, ccy, cchild, &pw)))
						{
							ret = Err_no_memory;
							goto cleanup;
						}
					}
				}
				break;
			}
		}

		if(cchild == NULL)
			goto nochild;

		reuse_input();
		pw.level = 1; /* it would be nice to have this recursive but alas
					   * this is basicly the way it was */
		p1 = NULL; 

		while((child = cchild->children) != NULL)
		{
			wait_input(ANY_INPUT);
			if(JSTHIT(KEYHIT))
			{
				ret = Err_not_found;
				reuse_input();
				goto cleanup;
			}

			cchild->height += pw.cheight;	/* let them go a bit below end */
			in_cchild = in_pblock(ccx, ccy, cchild);
			cchild->height -= pw.cheight;

			if(!(in_cchild))
			{
				unselect(&pw);
				--pw.level;  /*  = 0; */
				break;
			}

			gothit = 0;

			while(child)
			{
				cx = ccx + child->x;
				cy = ccy + child->y;

				if (in_pblock(cx, cy, child))
				{
					gothit = 1;
					if(p1 != child)
					{
						unselect(&pw);
						p1 = child;
						if(NULL == (pw.behind[1] 
						= (Raster *)save_behind(pw.screen
						, cx, cy, child->width, child->height)))
						{
							ret = Err_no_memory;
							goto cleanup;
						}
						if (!(child->flags & PULL_DISABLED))
							draw_quad((Raster *)pw.screen->viscel,
									pw.screen->SRED, cx, cy,
									child->width, child->height);
					}
					if(JSTHIT(MBPEN))
					{
						if(child->flags & PULL_DISABLED)
							ret = Err_disabled;
						else
							{
							*hitid = p1->id;
							ret = 1; /* yay! we got a hit */
							}
						wait_penup();
						goto cleanup;
					}
				}
				child = child->next;
			}
			if(!gothit)
			{
				unselect(&pw);
				p1 = NULL;
			}
		}

	nochild:
		if(wndo_dot_visible(&(mh->mw->w),icb.mx,icb.my))
			continue;
		ret = Err_not_found;
		reuse_input();
		break;
	}

cleanup:
	while(pw.level >= 0)
	{
		unselect(&pw);
		--pw.level;
	}
	return(ret);
}


int menu_dopull(Menuhdr *mh)
/* "feels" a pull initiated as (*menuhdr->domenu) in input loop */
{
SHORT selid = -1;
int ret;

	if(JSTHIT(KEYHIT))
	{
		if((ret = do_pullkeys(mh,&selid)) <= 0)
			return(ret);
	}
	else
	{
		if((ret = do_pullmouse(mh,&selid)) <= 0)
			return(ret);
	}
	if(mh->dodata != NULL)
		(*((VFUNC)(mh->dodata)))(mh,selid);
	return(ret);
}

