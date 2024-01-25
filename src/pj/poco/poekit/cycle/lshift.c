
#include "stdtypes.h"
#include "errcodes.h"
#include "rastcall.h"
#include "rastlib.h"
#include "menus.h"
#include "options.h"
#include "inkdot.h"

static Pixel lshift_dot(const Ink *inky, const SHORT x, const SHORT y)
{
Aa_ink_data *aid = inky->aid;

return(GET_DOT(aid->undo,x,y)<<1);
}

static void lshift_hline(const Ink *inky, SHORT x0, const SHORT y, SHORT width)
{
UBYTE buf[SBSIZE];
UBYTE *bpt;
SHORT i;
Aa_ink_data *aid = inky->aid;

GET_HSEG(aid->undo,buf,x0,y,width);
bpt = buf;
i = width;
while (--i >= 0)
	*bpt++ <<= 1;
PUT_HSEG(aid->screen,buf,x0,y,width);
}

RootInk rexlib_header = {
	{ REX_INK,0,NULL,NULL,NULL,NULL },
	INKINIT(
		NONEXT,
		"Lshift",
		INK_OPT,
		0,
		"Left shift image pixel by one (bitwise).",
		NO_SUBOPTS,
		lshift_dot,
		lshift_hline,
		NOSTRENGTH,
		FALSE,
		NO_MC,
		NO_FC,
		INK_NEEDS_UNDO,
	),
	NULL,
};
