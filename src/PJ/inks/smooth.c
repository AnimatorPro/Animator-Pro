#include "stdtypes.h"
#include "errcodes.h"
#include "rastcall.h"
#include "rastlib.h"
#include "menus.h"
#include "options.h"
#include "inkdot.h"


static Pixel ink_dot(Ink *inky, SHORT x, SHORT y)
{
Aa_ink_data *aid = inky->aid;
Rgb3 result;

	if (x >= aid->undo->width-1)
		return(GET_DOT(aid->undo,x,y));	/* ones on right edge stay constant */
	aid->true_blend(
		GET_DOT(aid->undo,x,y) + aid->undo->cmap->ctab,
		GET_DOT(aid->undo,x+1,y) + aid->undo->cmap->ctab,
		inky->strength,
		&result);
	return(aid->bclosest_col(&result, aid->ccount, inky->dither) );
}

static void ink_hline(const Ink *inky, SHORT x0, const SHORT y, SHORT width)
{
UBYTE buf[SBSIZE+1];
UBYTE *bpt = buf;
SHORT i = width;
UBYTE strength = inky->strength;
SHORT dither = inky->dither;
Aa_ink_data *aid = inky->aid;
int ccount = aid->ccount;
Rcel *undo = aid->undo;
Rgb3 result;
Rgb3 *ctab = undo->cmap->ctab;
void (*blend)(Rgb3 *dest, Rgb3 *source, UBYTE percent, Rgb3 *result) = 
																aid->true_blend;
int (*closestc)(Rgb3 *true_color, Rgb3 *cmap, int ccount) = aid->closestc;
int (*bclose)(Rgb3 *rgb,int count,SHORT dither) = aid->bclosest_col;


	if(x0 + width >= undo->width) /* duplicate last pixel if at end of line */
	{
		GET_HSEG(undo,buf,x0,y,width);
		buf[width] = buf[width-1];
	}
	else
	{
		GET_HSEG(undo,buf,x0,y,width+1);
	}
	while(--i >= 0)
	{
		blend(bpt[0] + ctab, bpt[1] + ctab, strength, &result);
		*bpt++ = bclose(&result, ccount, dither);
	}
	PUT_HSEG(aid->screen,buf,x0,y,width);
}


static Errcode init_smooth(Aa_ink_data *aid,  Ink_groups *igs);

RootInk rexlib_header = {
	{ REX_INK,0,NULL,NULL,NULL,NULL },
	INKINIT(
		NONEXT,
		RL_KEYTEXT("smooth_n")"Smooth",
		INK_OPT,
		0,
		RL_KEYTEXT("smooth_help"),
		NO_SUBOPTS,
		ink_dot,
		ink_hline,
		50,
		FALSE,
		NO_MC,
		NO_FC,
		(INK_NEEDS_UNDO),
	),
	init_smooth,
};

static Errcode init_smooth(Aa_ink_data *aid,  Ink_groups *igs)
/* fill in ink cache and stuff */
{
#define mi (rexlib_header.ink)
	mi.make_cashe = aid->make_bhash;
	mi.free_cashe = aid->free_bhash;
	return(Success);
#undef mi
}
