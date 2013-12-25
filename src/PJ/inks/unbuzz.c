#define REXLIB_INTERNALS
#include "stdtypes.h"
#include "errcodes.h"
#include "rastcall.h"
#include "rastlib.h"
#include "menus.h"
#include "options.h"
#include "inkdot.h"


#ifdef NOTYET
typedef struct aa_ink_data {
	Pixel ccolor;
	Pixel tcolor;
	Rcel *screen;		/* drawing screen */
	Rcel *undo;			/* undo screen */
	Rcel *cel;			/* cel */
	Rcel *alt;			/* alt screen */
	SHORT ccount;		/* # of colors in system */
	SHORT rmax,gmax,bmax;	/* max values of rgb components */
} Aa_ink_data;
#endif


#define UNBUZZ_OF(a,t,b,om,tm) \
	(((LONG)((LONG)(a + b) * om) + (LONG)(t * tm))/300)

static Pixel unbuzz_dot(const Ink *inky, const SHORT x, const SHORT y)
{
Aa_ink_data *aid = inky->aid;
Rcel *undo = aid->undo;
Rgb3 above, this, below;
Rgb3 *ctab;
LONG omult, tmult; 

	ctab = undo->cmap->ctab;
	this = ctab[pj__get_dot(undo,x,y)];

	if(y <= 0)
	{
		above = this;
	}
	else
	{
		above = ctab[pj__get_dot(undo,x,y-1)];
	}
	if(y >= undo->height - 1)
	{
		below = this; 
	}
	else
	{
		below = ctab[pj__get_dot(undo,x,y+1)];
	}
	omult = inky->strength; 	/* 0 to 100 */
	tmult = ((100 - omult)<<1) + 100; /* 100 to 300 */

	this.r = UNBUZZ_OF(above.r,this.r,below.r,omult,tmult);
	this.g = UNBUZZ_OF(above.g,this.g,below.g,omult,tmult);
	this.b = UNBUZZ_OF(above.b,this.b,below.b,omult,tmult);
	return((*aid->bclosest_col)(&this,COLORS,inky->dither));
}

static void unbuzz_hline(const Ink *inky, SHORT x0, const SHORT y, SHORT width)
{
Aa_ink_data *aid = inky->aid;
Pixel abuf[SBSIZE];
Pixel tbuf[SBSIZE];
Pixel bbuf[SBSIZE];
Pixel *pabove = abuf;
Pixel *pthis = tbuf;
Pixel *pbelow = bbuf;
Pixel *maxthis;
Rgb3 *ctab;
LONG omult, tmult; 
Rgb3 above, this, below;


	ctab = aid->undo->cmap->ctab;

	pj__get_hseg(aid->undo,pthis,x0,y,width);
	if(y <= 0)
		pabove = pthis;
	else
		pj__get_hseg(aid->undo,pabove,x0,y-1,width);

	if(y >= aid->undo->height - 1)
		pbelow = pthis;
	else
		pj__get_hseg(aid->undo,pbelow,x0,y+1,width);

	omult = inky->strength; 	/* 0 to 100 */
	tmult = ((100 - omult)<<1) + 100; /* 100 to 300 */

	maxthis = pthis + width;

	while(pthis < maxthis)
	{
		above = ctab[*pabove++];
		this = ctab[*pthis];
		below = ctab[*pbelow++];

		this.r = UNBUZZ_OF(above.r,this.r,below.r,omult,tmult);
		this.g = UNBUZZ_OF(above.g,this.g,below.g,omult,tmult);
		this.b = UNBUZZ_OF(above.b,this.b,below.b,omult,tmult);
		*pthis++ = ((*aid->bclosest_col)(&this,COLORS,inky->dither));
	}

	pj__put_hseg(aid->screen,tbuf,x0,y,width);
}

static Errcode init_unbuzz(Aa_ink_data *aid,  Ink_groups *igs);

RootInk rexlib_header = {
	INKINIT(
		NONEXT,
		RL_KEYTEXT("unbuzz_n")"UnBuzz",
		INK_OPT,
		0,
		RL_KEYTEXT("unbuzz_help"),
		NO_SUBOPTS,
		unbuzz_dot,
		unbuzz_hline,
		75,
		FALSE,
		NO_MC,
		NO_FC,
		INK_NEEDS_UNDO,
	),
	init_unbuzz,
};

static Errcode init_unbuzz(Aa_ink_data *aid,  Ink_groups *igs)
{
#define mi (rexlib_header.ink)
	mi.ot.options = igs->dstrength_group;
	mi.make_cashe = aid->make_bhash;
	mi.free_cashe = aid->free_bhash;
	return(Success);
#undef mi
}
