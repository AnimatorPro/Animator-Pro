/* globals.c - The really pervasive global data declarations. */

#include <stdio.h>
#include "jimk.h"
#include "fli.h"
#include "flicmenu.h"
#include "globals.str"

Vsettings vs = 
	{
	sizeof(Vsettings),
	VP_SETTING,
	0,	/* frame_ix - frame index */
	250,	/* ccolor - pen color */
	1,	/* zero_clear - color zero transparent in cel */
	3,	/* tool_ix - which drawing tool is active */
	0,	/* pen_width - 0 is one pixel */
	3,	/* big_pen - last selected pen larger than 1 pixel */
	{0,138,192,131,248,249,250,247}, /* inks wells */
	165, /* rmyoff root menu initial y offset */
	{32, 1, 0,  1},	/* color range */
	0, 6,	/* initial and toggle draw modes */
	{3, 7, 17, 0, 20, 5}, /* initial pen tools */
	/* draw, spray, fill, box, disk, separate */
	XMAX/4,YMAX/4,XMAX/2,YMAX/2,2,0,	/* initial zoom settings */
	50,	/* tint percent */
	MMD_PAINT, /* menu mode */
	0,	/* font_ix */
	50, 30, 220, 140,	/* text window dimensions */
	"",	/* drawer */
	globals_101 /* "untitled" */,	/* file */
	{0, 6, 5, 1, 7, 12, },	/* drawing modes in dm_menu */
	{"", },
	0,0,0,	/* text yoffset and text cursor position */
	0,0,0,	/* initial scroller_tops */
	6, 33,		/* star points , star ratio*/
	0,0,	/* make_mask, use_mask */
	0,0,8,8,0,	/* grid x,y,w,h, use_grid */
	50, /* dither threshold */
	60, 32,	/* air_speed, air_spread */
	0, /* dither */
	4,4,	/* quantization x and y */
	XMAX/2,YMAX/2,190,	/* radial gradient settings */
	1,  /* fillp  - fill polygons? */
	0,	/* color2 - 2 color polygons? */
	XMAX/2, YMAX/2,	/* marked point mkx mky */
	0,  /* multi */
	1,  /* closed_curve */
	16, /* transition frames */
	0,9,	/* start and stop of time segment */
	0,	/* browse type (0 = name) */
	0,  /* browse action (0 = load) */
	1,  /* auto cel fit colors = 1 */
	0,  /* separate rgb if 1, by color ix if 0 */
	10,	/* separate threshold */
	1,	/* ado tween */
	0,	/* ado ease */
	0,	/* ado pong */
	0,  /* ado mode == spin */
	2,  /* ado spin == turns */
	3,  /* ado size == both */
	0,  /* ado path == spline */
	0,  /* ado mouse == xy */
	360,  /* ado turn == 360 */
	0,  /* ado size mouse == proportional */
	{	/* move3 */
		NULL,
		{XMAX/2, YMAX/2, 0},
		{0, 0, 100},
		{0, 0, 0},
		0, 0,
		{XMAX/2, YMAX/2, 0},
		100, 100, 100, 100, 100, 100,
		{0, 0, 0},
	},
	0,	/* ado first */
	0,  /* ado reverse */
	1,  /* ado complete */
	0,  /* ado source */
	0,  /* ado outline */
	0,  /* spline tension (sp_tens) */
	0,  /* spline continuity (sp_cont) */
	0,  /* spline bias  (sp bias) */
	2,	/* time mode == 2 == to all */
		{
		{0, 0, 0},	/* menu black */
		{24, 24, 24}, /* menu grey */
		{42, 42, 42}, /* menu light */
		{58, 58, 58}, /* menu bright */
		{63, 0, 0},  /* menu red */
		{0, 0, 63},  /* menu extra */
		},
	0,				/* sep box */
	{0, 9, 99, 999},  /* time markers */
	{0, 10, 20, 30},	/* start of ranges */
	{9, 19, 29, 39},	/* stop of ranges */
	0, 		/* bframe_ix */
	0,		/* hls or rgb mode */
	0,		/* pal to - what portion of palette effected by remap */
	1,		/* use bundle */
	{
		{
		31,		/* bundle1 count */ 
		{ 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
		25,26,27,28,29,30,31, }
		},
		{
		30,		/* bundle2 count */ 
		{ 92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,
		111,112,113,114,115,116,117,118,119,120,121,},
		},
	},
	20,		/* cclose - closeness threshold */
	50,		/* ctint - strength of tinting in palette */
	0,		/* cycle_draw */
	0,		/* cycle draw ix */
	0,		/* title justify */
	0,		/* title scroll */
	0,		/* title menu */
	1,		/* palette fit */
	0,  /* path tension (pa_tens) */
	0,  /* path continuity (pa_cont) */
	0,  /* path bias  (pa bias) */
	0,  /* path closed */
	0,  /* ease out */
	50, /* max blend percent ... cblend */
	0,  /* zoom x4 */
	{
	50,50+0x80,50+0x80,50+0x80,50,		50,50,50,50,3,
	50,50,50,50,1,		50,50,50,50,50,
	50,50,50,50,50,		50+0x80,50,50,50,50,
	50,50,
	},
	};



int ivmode;
char init_drawer[71];

UBYTE sys_cmap[COLORS*3];
Video_form vf = { 0,0,XMAX,YMAX,Raster_line(XMAX),VGA_SCREEN,sys_cmap, };
