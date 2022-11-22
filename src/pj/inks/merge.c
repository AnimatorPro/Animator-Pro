#include "stdtypes.h"
#include "errcodes.h"
#include "rastcall.h"
#include "rastlib.h"
#include "menus.h"
#include "options.h"
#include "inkdot.h"

static Pixel merge_dot(Ink *inky, SHORT x, SHORT y)
{
Aa_ink_data *aid = inky->aid;
Rgb3 result;

if (aid->alt == NULL)
	return(CGET_DOT(aid->undo,x,y));
aid->true_blend(
	CGET_DOT(aid->undo,x,y) + aid->undo->cmap->ctab,
	CGET_DOT(aid->alt,x,y) + aid->alt->cmap->ctab,
	inky->strength,
	&result);
return(aid->bclosest_col(&result, aid->ccount, inky->dither) );
}

static Errcode init_merge(Aa_ink_data *aid,  Ink_groups *igs);

RootInk merge_ink_opt = {
	INKINIT(
		NONEXT,
		RL_KEYTEXT("merge_n")"Merge",
		INK_OPT,
		0,
		RL_KEYTEXT("merge_help"),
		NO_SUBOPTS,
		merge_dot,
		NULL,
		50,
		FALSE,
		NO_MC,
		NO_FC,
		(INK_NEEDS_UNDO|INK_NEEDS_ALT),
	),
	init_merge,
};

static Errcode init_merge(Aa_ink_data *aid,  Ink_groups *igs)
/* fill in ink cache and stuff */
{
#define mi (merge_ink_opt.ink)
mi.make_cashe = aid->make_bhash;
mi.free_cashe = aid->free_bhash;
mi.ot.options = igs->dstrength_group;
return(Success);
#undef mi
}
