#define WAITASK_C
#define INPUT_INTERNALS

#include "input.h"
#include "linklist.h"


typedef struct wtaskcb {
	Dlheader list;
	SHORT disables;
} Wtaskcb;

static Wtaskcb wtasks = {
	DLHEADER_INIT(wtasks.list),
	0,
};

void init_waitask(Waitask *wt,FUNC func,void *dat,USHORT flags)
{
	wt->doit = func;
	wt->data = dat;
	wt->flags = flags & ~(WT_ATTACHED);
}

void add_waitask(Waitask *wt)
{
	add_tail(&wtasks.list,&(wt->node));
	wt->flags |= WT_ATTACHED;
}
void rem_waitask(Waitask *wt)
{
	safe_rem_node(&(wt->node));
	wt->flags &= ~WT_ATTACHED;
}
void disable_wtasks(void)
{
	++wtasks.disables;
}
void enable_wtasks(void)
{
	--wtasks.disables;
}

void check_waitasks(void)
{
Waitask *wt;
Icb_savebuf *pushed;

	if(wtasks.disables > 0)
		return;

	if(NULL == (wt = (Waitask *)get_head(&wtasks.list)))
		return;

	pushed = icb.push;

	if((wt->flags & WT_KILLCURSOR) 
		&& icb.mset.on 
		&& icb.mcurs_up > 0)
	{
		icb.mset.on = 0;
		UNDRAWCURSOR();
		if((*wt->doit)(wt))
		{
			wt->flags &= ~WT_ATTACHED;
		}
		else /* put it back, try again */
		{
			add_tail(&wtasks.list,&(wt->node));
		}
		CHECK_POP_ICB(pushed);
		DRAWCURSOR();
		icb.mset.on = 1;
	}
	else
	{
		if((*wt->doit)(wt))
		{
			wt->flags &= ~WT_ATTACHED;
		}
		else
		{
			add_tail(&wtasks.list,&(wt->node));
		}
		CHECK_POP_ICB(pushed);
	}
}
