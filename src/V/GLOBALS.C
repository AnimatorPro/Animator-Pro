
/* Globals.c - all the really pervasive global variables. */

#include <stdio.h>
#include "jimk.h"
#include "fli.h"
#include "flicmenu.h"
#include "globals.str"

/* The Vsettings structure holds all of Vpaint's state info, all
   that's a fixed size and not too big at least.  The default_vs
   is where we start 1st time program's run.  */
Vsettings default_vs = 
	{
	sizeof(Vsettings),	/* size of self so can expand later in disk form */
	VP_SETTING,	/* Magic number */
	0,	/* frame_ix - frame index */
	250,	/* ccolor - pen color */
	1,	/* zero_clear - color zero transparent in cel */
	2,	/* tool_ix - which drawing tool is active */
	0,	/* pen_width - 0 is one pixel */
	3,	/* big_pen - last selected pen larger than 1 pixel */
	{0,138,192,131,248,249,250,247}, /* inks wells */
	165, /* rmyoff root menu initial y offset */
	{32, 1, 0,  1},	/* color range */
	0, 6,	/* initial and toggle draw modes */
	{2, 12, 18, 0, 21, 5}, /* initial pen tools */
	/* draw, spray, fill, box, disk, separate */
	XMAX/4,YMAX/4,XMAX/2,YMAX/2,2,0,	/* initial zoom settings */
	50,	/* tint percent */
	0, /* menu mode  - unused */
	0,	/* font_ix - always zero.  Not even used probably. */
	50, 30, 220, 140,	/* text window dimensions */
	"",	/* drawer */
	globals_101 /* "untitled" */,	/* file */
	{0, 6, 5, 1, 7, 12, },	/* drawing modes in dm_menu */
	{globals_102 /* "SYSTEM" */, },	/* font name */
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
	0,  /* sep_type */
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
	{	/* move3 - top of the optics motion stack */
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
		{22, 22, 22}, /* menu grey */
		{38, 38, 38}, /* menu light */
		{52, 52, 52}, /* menu bright */
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
	{	/* Ink strengths (hi bit set for dither on) */
	50,50+0x80,50+0x80,50+0x80,50,		50,50,50,50,3,
	50,50,50,50,1,		50,50,50,50,50,
	50,50,50,50,50,		50+0x80,50,50,50,50,
	50,50,
	},	
	1,  /* display coordinates */
	0,	/* randseed */
	};
Vsettings vs;		/* The settings used by program.  Default_vs gets copied
						here a lot. */

int ivmode;			/* Video mode Vpaint started in */
char init_drawer[71];	/* directory Vpaint started in */

/* 99% of the time an accurate representation of the hardware color
   registers. */
UBYTE sys_cmap[COLORS*3];

/* The screen structure for hardware frame buffer - Mr VGA himself */
Vscreen vf = { 0,0,XMAX,YMAX,Raster_line(XMAX),VGA_SCREEN,sys_cmap, };

/* This one  points to the visible  screen even if the above  one doesn't
   (which is  the case  in preview  */
Vscreen menu_vf = { 0,0,XMAX,YMAX,Raster_line(XMAX),VGA_SCREEN,sys_cmap, };


/* Color map and screen for undo buffer */
static UBYTE ucmap[COLORS*3];
Vscreen uf = { 0,0,XMAX,YMAX,Raster_line(XMAX),NULL,ucmap,};

/* This is the screen we actually draw on.  Usually it's equal to vf.
   Not the case in Zoom mode or during a preview though. */
Vscreen *render_form = &vf;

/* Mr. Alt screen */
Vscreen *alt_form;

/* Not just any cel, it's THE cel... */
Vcel *cel;

/* Mr. Mask */
PLANEPTR mask_plane;

/*  Stuff to keep track of text buffer */
char *text_buf;
int text_size;
int text_alloc;


/* Set to 1 when want to blit under something.  Be sure to set it
   back to 0 again when you're through. */
char under_flag;

/* list of disk drives that look like they're really on this machine */
char devices[26];
int dev_count;
