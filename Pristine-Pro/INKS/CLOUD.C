#include "stdtypes.h"
#include "errcodes.h"
#include "rastcall.h"
#include "rastlib.h"
#include "menus.h"
#include "options.h"
#include "inkdot.h"

int sqr_root(long i)
/*
** Sqr_root() - Return the integer square root the the long integer i
*/
{
unsigned long	mask;
long t;
unsigned short	result;
long		lolly;

for (mask = 1, lolly = i; mask < lolly; mask <<= 1, lolly >>= 1)
	;
result = 0;
while (mask)
	{
	t = result | mask;
	if ((t * t) <= i)
	    result = t;
	mask >>= 1;
	}
return ((int)result);
}

static int grey_strength;	/* grey level to match ink-strenght.  Precomputed
							 * by ink_cache function. */
static int strength_factor;	/* Partial factor precomputed by ink_cache */

static Pixel ink_dot(Ink *inky, SHORT x, SHORT y)
{
Aa_ink_data *aid = inky->aid;
Rcel *undo = aid->undo;
Rgb3 result;
Rgb3 *c;
int r,g,b;
int luminance;
int strength = inky->strength;

	c = undo->cmap->ctab + GET_DOT(undo,x,y);
	r = c->r;
	g = c->g;
	b = c->b;
	luminance = sqr_root(r*r + g*g + b*b);
	if (luminance > 0)
		{
		if ((r = r*strength_factor/(100*luminance)) >= RGB_MAX)
			r = RGB_MAX-1;
		if ((g = g*strength_factor/(100*luminance)) >= RGB_MAX)
			g = RGB_MAX-1;
		if ((b = b*strength_factor/(100*luminance)) >= RGB_MAX)
			b = RGB_MAX-1;
		result.r = r;
		result.g = g;
		result.b = b;
		}
	else
		result.r = result.g = result.b = grey_strength;

	return(aid->bclosest_col(&result, aid->ccount, inky->dither) );
}

static Errcode init_cloud(Aa_ink_data *aid,  Ink_groups *igs);

RootInk rexlib_header = {
	{ REX_INK,0,NULL,NULL,NULL,NULL },
	INKINIT(
		NONEXT,
		RL_KEYTEXT("cloud_n")"Cloud",
		INK_OPT,
		0,
		RL_KEYTEXT("cloud_help")"Force luminance to ink strength.  "
			"Makes picture look like it's uniformly lit and a bit washed out.",
		NO_SUBOPTS,
		ink_dot,
		NULL,
		50,
		FALSE,
		NO_MC,
		NO_FC,
		(INK_NEEDS_UNDO),
	),
	init_cloud,
};

static Errcode (*aid_make_bhash)(Ink *inky);	/* regular bhash */

static Errcode cloud_cashe(Ink *inky)
/* Precompute grey result for rgb 0,0,0 in ink_dot. */
{
int strength = inky->strength;

grey_strength = (RGB_MAX-1)*strength/100;
strength_factor = sqr_root(strength*strength*(RGB_MAX-1)*(RGB_MAX-1)*3);
return((*aid_make_bhash)(inky));
}

static Errcode init_cloud(Aa_ink_data *aid,  Ink_groups *igs)
/* fill in ink cache and stuff */
{
#define mi (rexlib_header.ink)
	aid_make_bhash = aid->make_bhash;
	mi.make_cashe = cloud_cashe;
	mi.free_cashe = aid->free_bhash;
	mi.ot.options = igs->dstrength_group;
	return(Success);
#undef mi
}
