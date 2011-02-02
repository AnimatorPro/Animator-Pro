#include "stdtypes.h"
#include "errcodes.h"
#include "rastcall.h"
#include "rastlib.h"
#include "menus.h"
#include "options.h"
#include "inkdot.h"

static Pixel sharpen_dot(Ink *inky, SHORT x, SHORT y)
{
Aa_ink_data *aid = inky->aid;
Rcel *undo = aid->undo;
Rgb3 *ctab = undo->cmap->ctab;
Rgb3 *ct;
Pixel local_pix[9];
static int local_vals[9] = {-1,-3,-1,-3,32,-3,-1,-3,-1};
int rtot,gtot,btot;
int i;
int lv;
Rgb3 result;
Rgb3 full;

if (x > 0 && x < undo->width-1 && y > 0 && y < undo->height-1)
	{
	rtot = gtot = btot = 0;
	(*(undo->lib->get_rectpix))((LibRast *)undo,local_pix,x-1,y-1,3,3);
	i = 9;
	while (--i >= 0)
		{
		ct = ctab + local_pix[i];
		lv = local_vals[i];
		rtot += ct->r*lv;
		gtot += ct->g*lv;
		btot += ct->b*lv;
		}
	rtot += 8;
	rtot >>= 4;
	gtot += 8;
	gtot >>= 4;
	btot += 8;
	btot >>= 4;
	if (rtot < 0)
		rtot = 0;
	if (rtot >= RGB_MAX)
		rtot = RGB_MAX-1;
	if (gtot < 0)
		gtot = 0;
	if (gtot >= RGB_MAX)
		gtot = RGB_MAX-1;
	if (btot < 0)
		btot = 0;
	if (btot >= RGB_MAX)
		btot = RGB_MAX-1;
	full.r = rtot;
	full.g = gtot;
	full.b = btot;
	aid->true_blend(
		ctab + local_pix[4],	/* get RGB value for middle pixel */
		&full,
		inky->strength,
		&result);
	return(aid->bclosest_col(&result, aid->ccount, inky->dither) );
	}
else
	return(GET_DOT(aid->undo,x,y));
}

static Errcode init_sharpen(Aa_ink_data *aid,  Ink_groups *igs);

RootInk rexlib_header = {
	{ REX_INK,0,NULL,NULL,NULL,NULL },
	INKINIT(
		NONEXT,
		RL_KEYTEXT("sharpen_n")"Sharpen",
		INK_OPT,
		0,
		RL_KEYTEXT("sharpen_help")"Image processing technique that helps focus "
			"a blurry picture.",
		NO_SUBOPTS,
		sharpen_dot,
		NULL,
		50,
		FALSE,
		NO_MC,
		NO_FC,
		(INK_NEEDS_UNDO|INK_NEEDS_ALT),
	),
	init_sharpen,
};

static Errcode init_sharpen(Aa_ink_data *aid,  Ink_groups *igs)
/* fill in ink cache and stuff */
{
#define mi (rexlib_header.ink)
mi.make_cashe = aid->make_bhash;
mi.free_cashe = aid->free_bhash;
mi.ot.options = igs->dstrength_group;
return(Success);
#undef mi
}
