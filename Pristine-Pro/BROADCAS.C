
/** broadcas.c  routines to handle global proadcast of menu and screen
 	refreshing "messages" **/

#include "input.h"
#include "broadcas.h"

static Waitask brotask;

static USHORT _colorwhy;
static Dlheader _color_reflist = DLHEADER_INIT(_color_reflist);
static USHORT _rmodewhy;
static Dlheader _rmode_reflist = DLHEADER_INIT(_rmode_reflist);

static Boolean do_brotask(Waitask *bt)
{
	if(_colorwhy)
		do_color_redraw(_colorwhy);
	if(_rmodewhy)
		do_rmode_redraw(_rmodewhy);
	return(1); /* remove self */
}
static void set_brotask()
{
	if(WT_ISATTACHED(&brotask))
		return;
	init_waitask(&brotask,do_brotask,NULL,WT_KILLCURSOR);
	add_waitask(&brotask);
}
static void do_redraw(Dlheader *list, USHORT why)
{
Dlnode *next;
Redraw_node *rn;

	for(rn = (Redraw_node *)(list->head);
		NULL != (next = ((Dlnode *)rn)->next);
		rn = (Redraw_node *)next )
	{
		if(why & rn->why)
			(*(rn->doit))(rn->data,why);
	}
}
void add_color_redraw(Redraw_node *rn)
{
	safe_rem_node(&rn->node);
	add_head(&_color_reflist,&rn->node);
}
void rem_color_redraw(Redraw_node *rn)
{
	safe_rem_node(&rn->node);
}
void do_color_redraw(USHORT why)
{
	do_redraw(&_color_reflist,why);
	_colorwhy = 0;
}
void set_color_redraw(USHORT why)
{
	_colorwhy |= why;
	set_brotask();
}
void add_rmode_redraw(Redraw_node *rn)
{
	safe_rem_node(&rn->node);
	add_head(&_rmode_reflist,&rn->node);
	_rmodewhy = 0;
}
void rem_rmode_redraw(Redraw_node *rn)
{
	safe_rem_node(&rn->node);
}
void do_rmode_redraw(USHORT why)
{
	do_redraw(&_rmode_reflist,why);
}
