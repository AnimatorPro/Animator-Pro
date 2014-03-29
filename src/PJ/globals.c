#define GLOBALS_C

/* Globals.c - all the really pervasive global variables. */

#include <stdio.h>
#include "jimk.h"
#include "brush.h"
#include "celmenu.h"
#include "composit.h"
#include "errcodes.h"
#include "filemenu.h"
#include "flx.h"
#include "inks.h"
#include "mask.h"
#include "options.h"
#include "pentools.h"
#include "timemenu.h"

/* The Vsettings structure holds all of Vpaint's state info, all
   that's a fixed size and not too big at least.  The default_vs
   is where we start 1st time program's run.  */

Vsettings default_vs =  {
    { sizeof(Vsettings), VSET_VS_ID, VSET_VS_VERS }, /* id for settings */	
	0,	/* frame_ix - frame index */
	250,	/* ccolor - pen color */
	2, /* zoomscale */
	FALSE, 	/* zoom_open */
	FALSE, 	/* use_brush */
	TRUE,   /* dcoor */

	TRUE,  	/* fillp  - fill polygons? */
	FALSE, 	/* color2 - 2 color polygons? */
	TRUE,  	/* closed_curve */
	FALSE, 	/* multi */
	TRUE,	/* clear_moveout - (move tool clears old area) */
	TRUE,	/* zero_clear - color zero transparent in cel */
	FALSE,  /* render_under */
	FALSE, 	/* render_one_color */
	TRUE,   /* fit_colors */
	FALSE,	/* make_mask */
	FALSE,  /* use_mask */
	TRUE,	/* pal_fit */
	FTP_FLIC,  /* file_type */
	{0,138,192,131,248,249,250,247}, /* inks wells */

	opq_INKID,	/* initial selected ink */
	{opq_INKID, vsp_INKID, 
	 tsp_INKID, rvl_INKID, 
	 soft_INKID, celt_INKID, 
	 anti_INKID, jmb_INKID }, /* initial inks */

	DRAW_PTOOL,	/* ptool_id - which drawing tool is active */
	{ DRAW_PTOOL, BOX_PTOOL,  /* initial pen tools */
	  POLYF_PTOOL, TEXT_PTOOL, 
	  SPRAY_PTOOL, FILL_PTOOL,
	  LINE_PTOOL, MOVE_PTOOL }, 

	0, 0, /* flicentx, flicenty */
	0, VS_MAXCOOR, /* quickcentx, quickcenty */
	VS_MAXCOOR/2, VS_MAXCOOR/2, /* zcentx, zcenty */
	VS_MAXCOOR/4*3, (VS_MAXCOOR/20)*6, /* zwincentx zwincenty */
	VS_MAXCOOR/2, VS_MAXCOOR/2, /* zwin width zwin height */
	50,	/* tint percent */
	{ 220, 140, 50, 30 },	/* text window rectangle */
	0,0,	/* text yoffset and text cursor position */
	0,0,	/* top_tool, top_ink - initial scroller_tops */
	6, 33,		/* star points , star ratio */
	0,0,VS_MAXCOOR/(320/8),VS_MAXCOOR/(200/8), /* grid x y w h */
	FALSE,	/* use_grid */
	50, /* dither threshold */
	60, 32,	/* air_speed, air_spread */
	4,4,	/* quantization x and y */
	VS_MAXCOOR/2,VS_MAXCOOR/2,  /* radial gradient center */
	(SHORT)((FLOAT)VS_MAXCOOR/5.5),	/* radial gradient radious rel (h+w)*2 */
	VS_MAXCOOR/2, VS_MAXCOOR/2,	/* marked point mkx mky */
	16, /* transition frames */
	0,9,	/* start and stop of time segment */
	0,  	/* browse_action (0 = load) */
	FALSE,  /* sep_rgb - by rgb if 1, by color ix if 0 */
	10,		/* sep_threshold */
	TRUE,	/* ado tween */
	FALSE,	/* ado ease */
	FALSE,  /* ease out */
	FALSE,	/* ado pong */
	FALSE,  /* ado reverse */
	TRUE,   /* ado complete */
	0,      /* ado source */
	FALSE, 	/* ado outline */

	0,  /* ado_mode == spin */
	2,  /* ado_spin == turns */
	3,  /* ado_size == both */
	0,  /* ado_path == spline */
	0,  /* ado_mouse == xy */
	0,  /* ado_szmouse == proportional */
	360,  /* ado_turn == 360 degrees */

#define WIDTH 320
#define HEIGHT 200
	{	/* move3 - top of the optics motion stack */
		NULL,
		{WIDTH/2, HEIGHT/2, 0},
		{0, 0, 100},
		{0, 0, 0},
		0, 0,
		{WIDTH/2, HEIGHT/2, 0},
		100, 100, 100, 100, 100, 100,
		{0, 0, 0},
	},
#undef WIDTH
#undef HEIGHT

	0,  	/* sp_tens */
	0,  	/* sp_cont */
	0,  	/* sp_bias */
	2,		/* time_mode == 2 == to all */

	0,				/* sep box */
	{0, 9, 99, 999},  /* time markers */
	{0, 10, 20, 30},	/* start of ranges */
	{9, 19, 29, 39},	/* stop of ranges */
	0, 		/* bframe_ix */
	0,		/* pal to - what portion of palette effected by remap */
	FALSE,		/* hls or rgb mode */
	1,		/* use_bun */
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
	0,		/* cdraw_ix */
	FALSE,	/* cycle_draw */
	0,		/* tit_just */
	0,		/* tit_scroll */
	0,		/* tit_move */
	0,  	/* pa_tens */
	0,  	/* pa_cont */
	0,  	/* pa_bias */
	0,  	/* pa_closed */
	50, 	/* cblend */
	30,     /* font_height */
	0,		/* box_bevel */
	{0, {0}},		/* redo - Empty record */
	0, 		/* font_type */
	0,		/* ped_yoff - poco editor cursor position */
	0,		/* ped_cursor_p - poco editor window y start */

	CELPT_MOVE,	/* cur_cel_tool */
	TRUE,		/* paste_inc_cel */
	FALSE,		/* cm_blue_last */
	FALSE,		/* cm_move_to_cursor */
	FALSE,		/* cm_streamdraw */
	0,			/* rot_grid */
	0,			/* tween_end */
	0,			/* tween_tool */
	50,			/* tween_magnet */
	FALSE,		/* tween_spline */

	CIRCLE_BRUSH, /* pen_brush_type */
	4,          /* circle_brush_size */
	4,          /* square_brush_size */
	4,          /* line_brush_size */
	0,   		/* line_brush_angle */
	5,			/* gel_brush_size */
	1, 			/* randseed */

	COMP_CUT, /* co_type */
	FALSE, /* co_still */
	FIT_TOA, /*  co_cfit */
	FALSE, /* co_reverse */
	TRUE,	/* co_matchsize */
	FALSE,	/* co_b_first */
	0,	  /* co_olap_frames */
	20,	  /* co_venetian_height */
	20,	  /* co_louver_width */
	20,   /* co_boxil_width */
	20,   /* co_boxil_height */

	/* expand settings */
	0, /* expand_x */
	0, /* expand_y */

	/* font spacing stuff */
	0, /* font_spacing */
	0, /* font_leading */
	1, /* font_unzag */
};
Vsettings vs;		/* The settings used by program.  Default_vs gets copied
						here a lot. */

Vbcb vb = /* yep, this is where it is */
{
	-1, /* video mode at -1  */
};

Vlcb vl;	/* local control block data, all zeros, see jimk.h */


Rcel *undof;  /* undo cel */

/* Not just any cel, it's THE cel... */
Flicel *thecel;

/* Mr. Mask */
Bitmap *mask_rast;    /* the new one !! both are set by alloc mask */


/* Set to 1 when want to blit under something.  Be sure to set it
   back to 0 again when you're through. */
char under_flag;

/* control block for time oriented menus */ 

Minitime_data flxtime_data = {
	first_frame,  /* first frame */
	prev_frame,   /* prev */
	go_time_menu, /* feel ix */
	next_frame,      /* next */
	mplayit,	  /* play it */
	last_frame,     /* last frame */
	go_time_menu, /* opt_all */
	qset_first_frame, /* opt_tsl_first */
	flx_get_frameix, /* get_frame_ix */
	flx_get_framecount,	 /* get_frame_count */
	NULL, /* clear_overlays used to clean up frame before seeking etc */
	NULL, /* draw_overlays used to restore overlays after seeking etc */
	flx_seek_frame_with_data,
	0,	  /* start with a clear stack */
	NULL, /* data */
};



/* these bracket scrub_cur_frame() and user flx seeks */
void flx_clear_olays(void)
{
	mini_clear_overlays(&flxtime_data);
}
void flx_draw_olays(void)
{
	mini_draw_overlays(&flxtime_data);
}
Boolean flx_olays_hidden(void)
{
	return(flxtime_data.olay_stack != 0);
}
