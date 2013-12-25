#define REXLIB_INTERNALS
#include "stdtypes.h"
#include "errcodes.h"
#include "rastcall.h"
#include "rastlib.h"
#include "menus.h"
#include "options.h"
#include "inkdot.h"

static Pixel is_jim;

static Pixel jim_dot(const Ink *inky, const SHORT x, const SHORT y)
{
	return(is_jim);
}

static void jim_hline(const Ink *inky, SHORT x0, const SHORT y, SHORT width)
{
	SET_HLINE(inky->aid->screen,is_jim,x0,y,width);
}

static Errcode make_jim_cashe(Ink *inky)
{
	if(pj_exists("H:\\JIM"))
		is_jim = 255;
	else
		is_jim = 0;
	return(Success);
}

Hostlib _a_a_stdiolib = { NULL, AA_STDIOLIB, AA_STDIOLIB_VERSION };
Hostlib _a_a_gfxlib = { &_a_a_stdiolib, AA_GFXLIB, AA_GFXLIB_VERSION };
Hostlib _a_a_syslib = { &_a_a_gfxlib, AA_SYSLIB, AA_SYSLIB_VERSION };

RootInk rexlib_header = {
	INKINIT(
		NONEXT,
		"Jim",
		INK_OPT,
		0,
		"Color 255 if file 'h:\\jim' exists, 0 otherwise. For testing.",
		NO_SUBOPTS,
		jim_dot,
		jim_hline,
		NOSTRENGTH,
		FALSE,
		make_jim_cashe,
		NO_FC,
		0,
	),
	NULL,
};

