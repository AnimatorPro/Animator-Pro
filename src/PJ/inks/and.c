
#include "stdtypes.h"
#include "errcodes.h"
#include "rastcall.h"
#include "rastlib.h"
#include "menus.h"
#include "options.h"
#include "inkdot.h"

static Pixel and_dot(Ink *inky, SHORT x, SHORT y)
{
Aa_ink_data *aid = inky->aid;

return(CGET_DOT(aid->undo,x,y) & aid->ccolor);
}

RootInk rexlib_header = {
	INKINIT(
		NONEXT,
		RL_KEYTEXT("and_n")"And",
		INK_OPT,
		0,
		RL_KEYTEXT("and_help"),
		NO_SUBOPTS,
		and_dot,
		NULL,
		NOSTRENGTH,
		FALSE,
		NO_MC,
		NO_FC,
		INK_NEEDS_UNDO|INK_NEEDS_COLOR,
	),
	NULL,
};

