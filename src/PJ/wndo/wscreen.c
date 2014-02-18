#include "errcodes.h"
#include "memory.h"
#include "ptrmacro.h"
#define WNDO_INTERNALS
#include "wndo.h"
#include "rastext.h"
#include "input.h"

static Rgb3 default_mc_ideals[NUM_MUCOLORS] = {
	{0, 0, 0},		/* menu black */
	{RGB_MAX/3, RGB_MAX/3, RGB_MAX/3}, 	/* menu grey */
	{RGB_MAX/3+RGB_MAX/5,RGB_MAX/3+RGB_MAX/5,RGB_MAX/3+RGB_MAX/5}, /* white */
	{RGB_MAX-RGB_MAX/5,RGB_MAX-RGB_MAX/5,RGB_MAX-RGB_MAX/5}, /* menu bright */
	{RGB_MAX, 0, 0}, /* menu red */
};
static Pixel default_mc_colors[NUM_MUCOLORS] = {
	FIRST_MUCOLOR,
	FIRST_MUCOLOR + 1,
	FIRST_MUCOLOR + 2,
	FIRST_MUCOLOR + 3,
	FIRST_MUCOLOR + 4,
};

void close_wscreen(Wscreen *s)
{
Wndo *w;

	if(s == NULL)
		return;

	cleanup_wait_wndo(s);
	while((w = (Wndo *)get_head(&s->wilist)) != NULL)
		_close_wndo(TOSTRUCT(Wndo,W_node,w));
	if(icb.input_screen == s)
	{
		set_input_screen(NULL);
		set_cursor(NULL);
		set_procmouse(NULL);
	}
	pj_gentle_free(s->wndo.rasts);
	pj_free(s);
}
Errcode open_wscreen(Wscreen **ps, WscrInit *si)

/* this will open a wscreen if the input_screen is NULL it will set the opened
 * one to the current input screen. close will set to input_screen to null if
 * it is equal to the input_screen.  Unless there are multiple screens up
 * one should not have to do explicit calls to set_input_screen() */
{
Errcode err;
Wscreen *s;
int allocsize;

	if(si->cel_a == NULL)
		return(Err_bad_input);

	if(si->flags & WS_NOMENUS) /* if flagged dont alloc menu part */
		allocsize = OFFSET(Wscreen,WS_FIRST_MENUFIELD);
	else
		allocsize = sizeof(Wscreen);

	if((s = pj_zalloc(allocsize)) == NULL)
		goto nomem_error;

	s->flags = si->flags & ~(WS_MUCOLORS_UP);

	init_list(&s->wilist);

	if((s->max_wins = si->max_wins) <= 0)
		s->max_wins = 1;
	else if(s->max_wins > MAX_WNDOS)
		s->max_wins = MAX_WNDOS;

	s->dispvd = si->disp_driver;
	s->wndovd = si->wndo_driver;

	/* assign cels for screen */

	s->viscel = si->cel_a;
	s->offcel = si->cel_b;

	/* and cursor */

	s->cursor = si->cursor;

	/* alloc rasts array 1 for screen and 1 for every window */

	if((s->wndo.rasts 
			= pj_zalloc((s->max_wins + 1) * sizeof(Raster *))) == NULL)
	{
		goto nomem_error;
	}

	/* set up screen window and raster 0 visible screen */
	s->wndo.rasts[SCREEN_RASTID] = (Raster *)(s->viscel); 
	s->wndo.rasts[NULL_RASTID] = &(s->wndo.behind);
	s->wndo.cmap = s->viscel->cmap;

	{
	WndoInit wi;

		clear_mem(&wi,sizeof(wi));

		copy_rectfields(s->viscel,&wi)
		wi.maxw = wi.width;
		wi.maxh = wi.height;
		wi.screen = s;  
		wi.flags = WNDO_BACKDROP;

		if((err = open_wndo(NULL,&wi)) < 0)
			goto error;
	}

	/* setup menu colors */

	s->mc_ideals = default_mc_ideals;
	copy_mem(s->mc_ideals,s->mc_lastrgbs,sizeof(s->mc_lastrgbs));
	copy_mem(default_mc_colors,s->mc_colors,sizeof(s->mc_colors));
	s->mc_lastalt = 0xFF;  /* this will force menucolors to be found */

	{
		s->mufont = get_sys_font();
	}

	if(icb.input_screen == NULL)
		set_input_screen(s);

	*ps = s;
	return(0);

nomem_error:
	err = Err_no_memory;
error:
	close_wscreen(s);
	*ps = NULL;
	return(err);
}
void set_input_screen(Wscreen *ws)
{
	icb.input_screen = ws;
}
void get_requestor_position(Wscreen *ws, SHORT width, SHORT height, 
						    Rectangle *pos)

/* returns center of requestor on screen centered on cursor and clipped to 
 * screen. If last and current position puts requestor under cursor,
 * Use the same position as the last one */
{
	pos->x = ws->last_req_pos.x + ((ws->last_req_pos.width - width)>>1);
	pos->y = ws->last_req_pos.y + ((ws->last_req_pos.height - height)>>1);
	pos->width = width;
	pos->height = height;

	if(!ptin_rect(&ws->last_req_pos,icb.sx,icb.sy)
		|| !ptin_rect(pos,icb.sx,icb.sy))
	{
		pos->x = icb.cx - (width>>1);
		pos->y = icb.cy - (height>>1);
	}

	/* clip to screen to make sure it is inside */	
	bclip_rect(pos,(Rectangle *)&(ws->wndo.RECTSTART));
	ws->last_req_pos = *pos; /* update position on screen struct */
}
void cancel_reqpos(Wscreen *screen)
/* voids last requestor position */
{
	screen->last_req_pos.width = screen->last_req_pos.height = 0;
}
