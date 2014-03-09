#define MUPARTS_INTERNALS
#include "pjbasics.h"
#include "menus.h"
#include "timesels.h"

extern Image cup, cdown, cleft, cright, csleft, cright2;

/****** minitime group *******/

#define MTD ((Minitime_data *)(b->group))

static void mt_feel_ix(Button *b)
{
	mtd_ix_dofeel(b,MTD->feel_ix);
}
static void mt_opt_all(Button *b)
{
	mtd_ix_dofeel(b,MTD->opt_all);
}
static SHORT ret_neg1(void *data)
{
	(void)data;
	return(-1);
}
static void mt_see_ix(Button *b)
{
SHORT ix;

	if(!MTD->get_frameix)
		MTD->get_frameix = ret_neg1; /* safety for the rest */

	ix = (*(MTD->get_frameix))(MTD->data) + 1;
	b->datme = &ix; 
	gbshortint(b);
}

#undef MTD

static Button mt_frameix_sel =  MB_INIT1(
	NONEXT,
	NOCHILD,
	27,9,18,0,
	NODATA,
	mt_see_ix,
	mt_feel_ix,
	mt_opt_all,
	NOGROUP,IXSEL_ID,
	NOKEY,
	MB_GHANG
	);

static Button mt_bot_sel =  MB_INIT1(
	&mt_frameix_sel,
	NOCHILD,
	9,9,66,0,
	&cdown,
	see_centimage,
	mt_feel_last,
	mt_opt_all,
	NOGROUP,0,
	NOKEY,
	MB_GHANG
	);

static Button mt_play_sel =  MB_INIT1(
	&mt_bot_sel,
	NOCHILD,
	11,9,55,0,
	&cright2,
	see_centimage,
	mt_feel_play,
	mt_opt_all,
	NOGROUP,0,
	DARROW,
	MB_GHANG
	);

static Button mt_fore1_sel =  MB_INIT1(
	&mt_play_sel,
	NOCHILD,
	10,9,45,0,
	&cright,
	see_centimage,
	mt_feel_next,
	mt_opt_all,
	NOGROUP,0,
	RARROW,
	MB_GHANG
	);

static Button mt_back1_sel =  MB_INIT1(
	&mt_fore1_sel,
	NOCHILD,
	10,9,8,0,
	&cleft,
	see_centimage,
	mt_feel_prev,
	mt_opt_all,
	NOGROUP,0,
	LARROW,
	MB_GHANG
	);

static Button mt_top_sel =  MB_INIT1(
	&mt_back1_sel,
	NOCHILD,
	9,9,0,0,
	&cup,
	see_centimage,
	mt_feel_first,
	mt_opt_all,
	NOGROUP,0,
	UARROW,
	MB_GHANG
	);

Button minitime_sel =  MB_INIT1(
	NONEXT,
	&mt_top_sel,
	79,9,0,0,
	NOTEXT,
	NOSEE,
	NOFEEL,
	mt_opt_all,
	NOGROUP,0,
	NOKEY,
	MB_GHANG
	);


