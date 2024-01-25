
#include "stdtypes.h"
#include "errcodes.h"
#include "rastcall.h"
#include "rastlib.h"
#include "menus.h"
#include "options.h"
#include "inkdot.h"

static UBYTE dif_table[256];
static UBYTE clip_table[256*3];
static UBYTE *clip = clip_table + 256;

static UBYTE quantize(int a)
{
int i;

if (a <= 2)
	return a;
for (i=2; i<=128; i<<=1)
	{
	if (a <= i + i*7/12)
		return i;
#ifdef FUZZ
	if (a <= i + 2*i/3)
		return i;
#endif /* FUZZ */
#ifdef LOW
	if (a < i + i/2)
		return i;
#endif /* LOW */
#ifdef MED
	if (a < i + i/4)
		return i;
#endif /* MED */
#ifdef SHARP
	if (a <= i)
		return i;
#endif /* SHARP */
	}
return 255;
}

static Errcode init_quantize(Aa_ink_data *aid,  Ink_groups *igs)
{
int i;
for (i=0; i<256; ++i)
	{
	dif_table[i] = quantize(i);
	}
for (i=0; i<256; ++i)
	clip_table[i] = 0;
for (i=2*256; i<3*256; ++i)
	clip_table[i] = 255;
for (i=0; i<256; ++i)
	clip[i] = i;
return Success;
}

static Pixel quantize_dot(const Ink *inky, const SHORT x, const SHORT y)
{
Aa_ink_data *aid = inky->aid;
int c = GET_DOT(aid->undo, x, y);
int lastc;
int difc;

if (x == 0)
	return c;
lastc = GET_DOT(aid->screen, x-1, y);
difc = c - lastc;
if (difc >= 0)
	return clip[lastc + dif_table[difc]];
else
	return clip[lastc - dif_table[-difc]];
}

static void quantize_hline(const Ink *inky, SHORT x0, const SHORT y, SHORT width)
{
UBYTE buf[SBSIZE];
UBYTE last_plotted;
int c;
int lastc;
int difc;
SHORT i;
Aa_ink_data *aid = inky->aid;

	/* 1st pixel in line we leave alone. */
if (x0 == 0)
	lastc = GET_DOT(aid->screen,x0,y);
else
	lastc = GET_DOT(aid->screen,x0-1,y);
GET_HSEG(aid->undo,buf,x0,y,width);
for (i=0; i<width; ++i)
	{
	c = buf[i];
	difc = c - lastc;
	if (difc >= 0)
		lastc = lastc + dif_table[difc];
	else
		lastc =  lastc - dif_table[-difc];
	buf[i] = lastc = clip[lastc];
	}
PUT_HSEG(aid->screen,buf,x0,y,width);
}

RootInk quantize_ink_opt = {
	INKINIT(
		NONEXT,
		RL_KEYTEXT("quantize_n")"Quantize",
		INK_OPT,
		0,
		RL_KEYTEXT("or_help")"Set pixel to be a discrete distance from neighbor",
		NO_SUBOPTS,
		quantize_dot,
		quantize_hline,
		NOSTRENGTH,
		FALSE,
		NO_MC,
		NO_FC,
		INK_NEEDS_UNDO,
	),
	init_quantize,
};
