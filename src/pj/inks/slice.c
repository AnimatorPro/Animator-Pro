
#include "stdtypes.h"
#include "errcodes.h"
#include "rastcall.h"
#include "rastlib.h"
#include "menus.h"
#include "options.h"
#include "inkdot.h"

/********** shatter ink stuff ********************/
static Pixel slice_dot(const Ink *inky, const SHORT x, const SHORT y)
{
short ny;
Aa_ink_data *aid = inky->aid;

if (x&1)
	ny = y+inky->strength;
else
	ny = y-inky->strength;
if (ny < 0 || ny >= aid->screen->height)
	return(aid->tcolor);
else
	return(GET_DOT(aid->undo,x,ny));
}

static Errcode init_slice(Aa_ink_data *aid,  Ink_groups *igs);

RootInk slice_ink_opt = {
	INKINIT(
		NONEXT,
		RL_KEYTEXT("slice_n")"Slice",
		INK_OPT,
		0,
		RL_KEYTEXT("slice_help"),
		NO_SUBOPTS,
		slice_dot,
		NULL,
		1,
		FALSE,
		NO_MC,
		NO_FC,
		INK_NEEDS_UNDO,
	),
	init_slice,
};

static Errcode init_slice(Aa_ink_data *aid,  Ink_groups *igs)
/* fill in ink cache and stuff */
{
#define mi (slice_ink_opt.ink)
mi.ot.options = igs->strength_group;
return(Success);
#undef mi
}
