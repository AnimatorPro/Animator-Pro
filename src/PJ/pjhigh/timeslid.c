#define MUPARTS_INTERNALS
#include "pjbasics.h"
#include "reqlib.h"
#include "timesels.h"

extern Image cup, cdown, cleft, cright, csleft, cright2;

static SHORT frame_ix;
static Qslider frame_sl = QTSL_INIT1(0, 0, &frame_ix, 1, NULL, leftright_arrs,
									 TRUE );

static void see_time_slider(Button *b)
{
Minitime_data *mtd = b->group;

	frame_ix = (*(mtd->get_frameix))(mtd->data);
	frame_sl.max = (*(mtd->get_framecount))(mtd->data) - 1;
	see_qslider(b);
}
static void feel_time_slider(Button *b)
{
	feel_qslider(b);
	mini_seek_frame(b->group,frame_ix);
}

static void jump_to_frame(Button *b)
{
short x;
Minitime_data *mtd = b->group;
SHORT cur_ix;
SHORT frame_count;

	if(!mtd->seek_frame)
		return;

	cur_ix = (*(mtd->get_frameix))(mtd->data);
	frame_count  = (*(mtd->get_framecount))(mtd->data);

	hide_mp();
	if(in_right_arrow(b))
	{
		x = 1;
		if(soft_qreq_number(&x,1,frame_count,"move_ahead"))
			x = cur_ix + x;
		else
			goto OUT;
	}
	else if (in_left_arrow(b))
	{
		x = 1;
		if(soft_qreq_number(&x,1,frame_count,"move_back"))
			x = cur_ix - x;
		else
			goto OUT;
	}
	else
	{
		x = cur_ix + 1;
		if(soft_qreq_number(&x,1,frame_count,"jump_frame"))
			x -= 1;
		else
			goto OUT;
	}
	mini_seek_frame(mtd,x);
OUT:
	show_mp();
}
static void opt_tsl_first(Button *b)
{
Minitime_data *mtd = (Minitime_data*)(b->group);

	if(mtd->opt_tsl_first)
		(*mtd->opt_tsl_first)(mtd->data);
}


static Button tsl_frame_sl_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	157,11,11,0,
	&frame_sl,
	see_time_slider,
	feel_time_slider,
	jump_to_frame,
	NOGROUP,IXSEL_ID,
	NOKEY,
	MB_GHANG
	);
static Button tsl_next_sel = MB_INIT1(
	&tsl_frame_sl_sel,
	NOCHILD,
	1,1,-1,-1,
	NOTEXT,
	NOSEE,
	mt_feel_next,
	NOOPT,
	NOGROUP,0,
	RARROW,
	MB_GHANG
	);
static Button tsl_prev_sel = MB_INIT1(
	&tsl_next_sel,
	NOCHILD,
	1,1,-1,-1,
	NODATA,
	NOSEE,
	mt_feel_prev,
	NOOPT,
	NOGROUP,0,
	LARROW,
	MB_GHANG
	);
static Button tsl_play_sel = MB_INIT1(
	&tsl_prev_sel,
	NOCHILD,
	13,11,169,0,
	&cright2,
	see_centimage,
	mt_feel_play,
	NOOPT,
	NOGROUP,0,
	DARROW,
	MB_GHANG
	);
static Button tsl_down_sel = MB_INIT1(
	&tsl_play_sel,
	NOCHILD,
	11,11,181,0,
	&cdown,
	see_centimage,
	mt_feel_last,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	MB_GHANG
	);
Button timeslider_sel = MB_INIT1(
	&tsl_down_sel,
	NOCHILD,
	13,11,0,0,
	&cup,
	see_centimage,
	mt_feel_first,
	opt_tsl_first,
	NOGROUP,0,
	UARROW,
	MB_GHANG
	);

