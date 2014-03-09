#define MUPARTS_INTERNALS
#include "pjbasics.h"
#include "timesels.h"

#define MTD ((Minitime_data *)(b->group))

static void redraw_ix(Button *b)
{
	draw_buttontop(find_button(b,IXSEL_ID));
}
void mtd_ix_dofeel(Button *b, void (*feeler)(void *))
{
Minitime_data *mtd = b->group;
SHORT oix;

	if(!feeler)
		return;
	oix = (*mtd->get_frameix)(mtd->data);
	(*feeler)(mtd->data);
	if(oix != (*mtd->get_frameix)(mtd->data))
		redraw_ix(b);
}
void mini_clear_overlays(Minitime_data *mtd)
{
	if(mtd->olay_stack == 0)
	{
		if(mtd->clear_overlays)
			(*(mtd->clear_overlays))(mtd->data);
	}
	++mtd->olay_stack;
}
void mini_draw_overlays(Minitime_data *mtd)
{
	if(--mtd->olay_stack == 0)
	{
		if(mtd->draw_overlays)
			(*(mtd->draw_overlays))(mtd->data);
	}
}
void mtd_clear_dofeel(Button *b, void (*feeler)(void *))
{
	if(!feeler)
		return;
	mini_clear_overlays(MTD);
	mtd_ix_dofeel(b,feeler);
	mini_draw_overlays(MTD);
}
void mt_feel_prev(Button *b)
{
	mtd_clear_dofeel(b,MTD->prev_frame);
}
void mt_feel_next(Button *b)
{
	mtd_clear_dofeel(b,MTD->next_frame);
}
void mt_feel_first(Button *b)
{
	mtd_clear_dofeel(b,MTD->first_frame);
}
void mt_feel_play(Button *b)
{
	mtd_clear_dofeel(b,MTD->play_it);
}
void mt_feel_last(Button *b)
{
	mtd_clear_dofeel(b,MTD->last_frame);
}
void update_time_sel(Button *b)

/* called with hanger sel */
{
	if(get_button_wndo(b) != NULL)
		draw_button(find_button(b->children,IXSEL_ID));
}

/*** for when called without button *****/


void mini_seek_frame(Minitime_data *mtd,SHORT ix)
{
	mini_clear_overlays(mtd);
	(*(mtd->seek_frame))(ix,mtd->data);
	mini_draw_overlays(mtd);
}
static void mtd_clear_call(Minitime_data *mtd, VFUNC func)
{
	mini_clear_overlays(mtd);
	(*func)(mtd->data);
	mini_draw_overlays(mtd);
}
void mini_playit(Minitime_data *mtd)
{
	mtd_clear_call(mtd,mtd->play_it);
}
void mini_prev_frame(Minitime_data *mtd)
{
	mtd_clear_call(mtd,mtd->prev_frame);
}
void mini_next_frame(Minitime_data *mtd)
{
	mtd_clear_call(mtd,mtd->next_frame);
}
void mini_first_frame(Minitime_data *mtd)
{
	mtd_clear_call(mtd,mtd->first_frame);
}
void mini_last_frame(Minitime_data *mtd)
{
	mtd_clear_call(mtd,mtd->last_frame);
}
