
#include "stdtypes.h"
#include "errcodes.h"
#include "rastcall.h"
#include "rastlib.h"
#include "menus.h"
#include "options.h"
#include "inkdot.h"

static Pixel or_dot(const Ink *inky, const SHORT x, const SHORT y)
{
Aa_ink_data *aid = inky->aid;

return(GET_DOT(aid->undo,x,y) | aid->ccolor);
}

static void or_hline(const Ink *inky, SHORT x0, const SHORT y, SHORT width)
{
UBYTE buf[SBSIZE];
UBYTE *bpt;
SHORT i;
Aa_ink_data *aid = inky->aid;

GET_HSEG(aid->undo,buf,x0,y,width);
bpt = buf;
i = width;
while (--i >= 0)
	*bpt++ |= aid->ccolor;
PUT_HSEG(aid->screen,buf,x0,y,width);
}

RootInk rexlib_header = {
	INKINIT(
		NONEXT,
		RL_KEYTEXT("or_n")"Or",
		INK_OPT,
		0,
		RL_KEYTEXT("or_help"),
		NO_SUBOPTS,
		or_dot,
		or_hline,
		NOSTRENGTH,
		FALSE,
		NO_MC,
		NO_FC,
		INK_NEEDS_UNDO|INK_NEEDS_COLOR,
	),
	NULL,
};
