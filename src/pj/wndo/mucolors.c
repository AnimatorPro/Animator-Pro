#include <string.h>
#include "memory.h"
#include "ptrmacro.h"
#include "input.h"
#include "wndo.h"

/* min difference color threshold */
#define SEE_T (24*24)

Boolean visible_mucolors(Cmap *cmap, Pixel *mucolors)
/* Are colors distinct enough from each other? */
{
int i, j;
Rgb3 *a;
Rgb3 mycolors[NUM_MUCOLORS];

	for (i=(NUM_MUCOLORS - 1); i >= 0; --i)
		get_color_rgb(mucolors[i],cmap,&mycolors[i]);

	for (i=(NUM_MUCOLORS - 1); i >= 0; --i)
	{
		if(i == MC_RED) /* let menu red be invisible if it wants... */
			continue;

		a = &mycolors[i];
		for (j=(NUM_MUCOLORS - 1); j >= 0; --j)
		{
			if(j == MC_RED) /* let menu red be invisible if it wants... */
				continue;

			if (i != j)
			{
				if (color_dif(a,&mycolors[j]) < SEE_T)
					return(0);
			}
		}
	}
	return(1);
}
static void f_colors(Wscreen *s, Cmap *cmap, Pixel *mucolors, 
				     Boolean set_lasts, Boolean force_unique)
{
int i;
Pixel ignores[NUM_MUCOLORS];

	for (i=0; i<NUM_MUCOLORS; ++i)
	{
		if(force_unique)
		{
			*mucolors = closestc_excl(&(s->mc_ideals[i]),cmap->ctab,
									  cmap->num_colors, ignores, i);
			ignores[i] = *mucolors;
		}
		else
		{
			*mucolors = closestc(&(s->mc_ideals[i]),cmap->ctab,
								  cmap->num_colors);
		}
		if(set_lasts)
			s->mc_lastrgbs[i] = cmap->ctab[*mucolors];
		++mucolors;
	}
}
Boolean has_menu_colors(Cmap *cmap, Wscreen *s)
/* returns true if the colormap has a set of visible menu colors for the 
 * screen ideal settings */
{
Pixel mc_colors[NUM_MUCOLORS];

	f_colors(s, cmap, mc_colors, FALSE, FALSE);
	return(visible_mucolors(cmap, mc_colors));
}
static void f_mucolors(Wscreen *s,Boolean force_unique)
{
	f_colors(s, s->viscel->cmap, s->mc_colors, TRUE, force_unique);
}
static void check_mucmap(Wscreen *s)
{
static Pixel mcolors[] = { FIRST_MUCOLOR,
						   FIRST_MUCOLOR + 1,
						   FIRST_MUCOLOR + 2,
						   FIRST_MUCOLOR + 3,
						   FIRST_MUCOLOR + 4 };

	if(!visible_mucolors(s->viscel->cmap, s->mc_colors))
	{
		s->mc_alt = (UBYTE)(~(((UBYTE)~0) << NUM_MUCOLORS));
		pj_set_colors(s->viscel, FIRST_MUCOLOR, NUM_MUCOLORS, 
					(UBYTE *)(s->mc_ideals));
		copy_mem(mcolors, s->mc_colors, sizeof(s->mc_colors));
	}
	else
		s->mc_alt = 0;

	s->mc_lastalt = s->mc_alt;
	s->flags |= WS_MUCOLORS_UP;
}
void uncheck_mucmap(Wscreen *s)
{
Rgb3 *colors;

	if(s->mc_alt)
	{
		colors = &(s->viscel->cmap->ctab[FIRST_MUCOLOR]);
		pj_set_colors(s->viscel, FIRST_MUCOLOR, NUM_MUCOLORS, (UBYTE *)colors);
		f_mucolors(s,TRUE);
		++s->mc_csetid; /* assume it has changed */
	}
	s->mc_alt = s->mc_lastalt = 0;
	s->flags &= ~(WS_MUCOLORS_UP);
}
Boolean lastmuc_changed(Wscreen *s)
{
Rgb3 *this;
Rgb3 *last;
Rgb3 *ctab;
int i;

	if(s->mc_lastalt)
		return(s->mc_alt != s->mc_lastalt);

	ctab = s->viscel->cmap->ctab;
	last = s->mc_lastrgbs;

	for (i=0; i<NUM_MUCOLORS; ++i)
	{
		this = ctab + s->mc_colors[i];
		if( last->r != this->r
			|| last->g != this->g
			|| last->b != this->b )
		{
			return(1);
		}
		++last;
	}
	return(0);
}
Boolean find_mucolors(Wscreen *ws)

/* returns true and installs window refresh task if color set was changed */
{
Pixel ocolors[NUM_MUCOLORS];
UBYTE oalt;

	oalt = ws->mc_alt;
	copy_mem(ws->mc_colors,ocolors,sizeof(ocolors));
	f_mucolors(ws,FALSE);
	check_mucmap(ws);
	if(memcmp(&ocolors,&ws->mc_colors,sizeof(ocolors))
		|| oalt != ws->mc_alt )
	{
		++ws->mc_csetid;
		set_refresh(ws); /* note: will not recurse cause of ATTACHED test */
		return(1);
	}
	return(0);
}
void try_mucolors(Wscreen *s)
/* if things are not up it will try for the best ones possible */
{
	if(s->flags & WS_MUCOLORS_UP) 
		find_mucolors(s);
	else
	{
		f_mucolors(s,TRUE);
		++s->mc_csetid; /* Just to make sure */
	}
}
void set_new_mucolors(Rgb3 *new_ideals, Wscreen *s)

/* loads a new set of menu colors into a screen's mc_ideals array and refreshes
 * all windows that need them */
{
SHORT was_up;

	was_up = (s->flags & (WS_MUCOLORS_UP));
	uncheck_mucmap(s); /* in case they are up */
	pj_copy_bytes(new_ideals, s->mc_ideals, sizeof(Rgb3)*NUM_MUCOLORS );
	s->flags |= was_up;
	try_mucolors(s);
	++s->mc_csetid; /* force redraw in any case! */
	set_refresh(s); /* note: will not recurse cause of ATTACHED test */
}

/**** menu window refresh "task" called from within input loop ******/

static int rtask_func(Waitask *wt)
{
Boolean need_colors;
register Wndo *w;
Wscreen *ws;
Dlnode *next;

	ws = wt->data;

	/* get new color set */

	need_colors = TRUE;

	/* redraw all windows that need it */

	for(w = (Wndo *)(ws->wilist.head);
		(next = ((Dlnode *)w)->next) != NULL;
		w = (Wndo *)next )
	{
		w = TOSTRUCT(Wndo,W_node,w);

		if((w->flags & (WNDO_HIDDEN|WNDO_MUCOLORS)) != WNDO_MUCOLORS)
			continue;

		if(need_colors) /* don't do if done already */
		{
			find_mucolors(ws); 
			need_colors = FALSE;
		}

		if(w->mc_csetid != ws->mc_csetid)
			redraw_wndo(w);
	}
	ws->flags &= ~WS_REFRESH_SET;
	return(1); /* done with it */
}

static Waitask reftask;

static void add_reftask(Wscreen *ws)
{
	if(WT_ISATTACHED(&reftask))
		return;
	init_waitask(&reftask,rtask_func,ws,WT_KILLCURSOR);
	add_waitask(&reftask);
}
void set_refresh(Wscreen *ws)

/* installs a task node to check colors and refresh windows and remove 
 * itself */
{
	if(ws->flags & WS_REFRESH_SET)
		return;
	ws->flags |= WS_REFRESH_SET;
	if(ws->refresh_disables)
		return;
	add_reftask(ws);
}
void disable_wrefresh(Wscreen *ws)
{
	if((++ws->refresh_disables) != 1 || !(ws->flags & WS_REFRESH_SET))
		return;
	rem_waitask(&reftask);
}
void enable_wrefresh(Wscreen *ws)
{
	if((--ws->refresh_disables) > 0 || !(ws->flags & WS_REFRESH_SET))
		return;
	add_reftask(ws);
}
void init_wrefresh(Wscreen *ws)

/* call this if you think window refresh stack has gotten screwed. Call from
 * startup level of program. may be called at init too */
{
	ws->refresh_disables = 0;
	ws->flags &= ~(WS_REFRESH_SET);
	rem_waitask(&reftask);
}
