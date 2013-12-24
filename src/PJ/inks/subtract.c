
#include "stdtypes.h"
#include "errcodes.h"
#include "rastcall.h"
#include "rastlib.h"
#include "menus.h"
#include "options.h"
#include "inkdot.h"

static Pixel subtract_dot(Ink *inky, SHORT x, SHORT y)
{
Aa_ink_data *aid = inky->aid;

return((CGET_DOT(aid->undo,x,y) - aid->ccolor)&COLORS-1);
}

RootInk rexlib_header = {
	{ REX_INK,0,NULL,NULL,NULL,NULL },
	INKINIT(
		NONEXT,
		RL_KEYTEXT("minus_n")"Minus",
		INK_OPT,
		0,
		RL_KEYTEXT("minus_help"),
		NO_SUBOPTS,
		subtract_dot,
		NULL,
		NOSTRENGTH,
		FALSE,
		NO_MC,
		NO_FC,
		INK_NEEDS_COLOR|INK_NEEDS_UNDO,
	),
	NULL,
};
