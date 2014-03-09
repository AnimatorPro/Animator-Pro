#include "ptrmacro.h"
#include "rastcurs.h"

/****** a staticly defined default crosshair cursor ******/

#define _ 0
#define B (CURS_MC0+MC_BRIGHT)
#define M (CURS_MC0+MC_BLACK)

static UBYTE xhair_pixels[] = {

	_,_,_,_,_,_,_,_,_,B,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,B,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,B,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,B,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,B,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,B,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,M,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
	B,B,B,B,B,B,M,_,_,_,_,_,M,B,B,B,B,B,B,
	_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,M,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,B,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,B,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,B,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,B,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,B,_,_,_,_,_,_,_,_,_,
	_,_,_,_,_,_,_,_,_,B,_,_,_,_,_,_,_,_,_,
};

#undef _
#undef B
#undef M


static Cursorcel _default_cursor;
static Pixel vsave[DFLT_CURS_HT];
static Pixel hsave[DFLT_CURS_WID];
static Short_xy spos;
#define XHOT 9
#define YHOT 9

static void show_default_cursor(Cursorhdr *ch)
{
SHORT cx, cy;
Rcel *screen;
(void)ch;

	screen = icb.input_screen->viscel;
	cx = (spos.x = icb.cx) - XHOT;
	cy = (spos.y = icb.cy) - YHOT;
	pj_get_hseg(screen,hsave,cx,spos.y,DFLT_CURS_WID);
	pj_get_vseg(screen,vsave,spos.x,cy,DFLT_CURS_HT);
	procblit(&_default_cursor,0,0,screen,cx,cy,DFLT_CURS_HT,DFLT_CURS_WID,
			 tbli_xlatline,get_cursor_xlat());
}
static void hide_default_cursor(Cursorhdr *ch)
{
Rcel *screen;
(void)ch;

	screen = icb.input_screen->viscel;
	pj_put_hseg(screen,hsave,spos.x-XHOT,spos.y,DFLT_CURS_WID);
	pj_put_vseg(screen,vsave,spos.x,spos.y-YHOT,DFLT_CURS_HT);
}
Rastcursor *get_default_cursor(void)
{
static Rastcursor dcurs = {
 	{ show_default_cursor, hide_default_cursor, NULL },
	&_default_cursor,
	NULL 
};

	_default_cursor.width = DFLT_CURS_WID;
	_default_cursor.height = DFLT_CURS_HT;
	_default_cursor.pdepth = 8;
	_default_cursor.aspect_dx = _default_cursor.aspect_dy = 1;
	pj_build_bytemap((Rasthdr *)&_default_cursor,
					(Raster *)&_default_cursor, xhair_pixels);
	_default_cursor.x = XHOT;
	_default_cursor.y = YHOT;
	return(&dcurs);
}

