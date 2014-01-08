#ifndef JIMK_H
#define JIMK_H

#ifndef PJBASICS_H
	#include "pjbasics.h"
#endif

#ifndef GFX_H
	#include "gfx.h"
#endif

#ifndef ASM_H
	#include "asm.h"
#endif

#ifndef MARQI_H
	#include "marqi.h"
#endif

#ifndef FLI_H
	#include "fli.h"
#endif

#ifndef FLX_H
	#include "flx.h"
#endif

#ifndef FLICEL_H
	#include "flicel.h"
#endif

#ifndef OPTIONS_H
	#include "options.h"
#endif

#ifndef REDO_H
	#include "redo.h"
#endif

#ifndef TIMESELS_H
	#include "timesels.h"
#endif

#ifndef IMATH_H
	#include "imath.h"
#endif

#ifndef PICFILE_H
	#include "picfile.h"
#endif

#ifndef REQLIB_H
	#include "reqlib.h"
#endif

#ifndef VSETFILE_H
	#include "vsetfile.h"
#endif

void return_to_main(Errcode code);

enum maincodes {
	RESET_SCREEN_SIZE = 1, /* reset screen size settings and flx unchanged */
	RESET_DEFAULT_FLX = 2, /* reset to default flx leave size same */
	RESET_NEW_SIZE = 3,    /* reset default flx and change size */
	KILL_NEW_SIZE = 4,	   /* kill current reset size but not settings */

	RESTART_VPAINT = 9,
	EXIT_SYSTEM = 10,
	QUIT_SYSTEM = 11,
};


/************* menu and window ids *************/

enum muids {
	QUICK_MUID		= 1,	/* quick menu id */
	FILE_MUID		= 2,	/* filemenu id */
	TITLE_MUID		= 3,	/* title menu id */
	OPTIC_MUID		= 4,	/* optics menu id */
	PALETTE_MUID	= 5,	/* palette menu id */
	MULTI_MUID		= 6,	/* multi menu id */
	OPT_MUID		= 7,	/* options menu id */
	FILEQ_MUID		= 8,	/* fileq menu id */
	MAINP_MUID		= 9,	/* main pull menu id */
	A3DP_MUID		= 10,	/* a3d pull menu id */
	PALP_MUID		= 11,	/* palette pull menu id */
	ZOOM_WNDOID 	= 12,	/* zoom window "menu" */
	FLI_WNDOID		= 13,	/* fli window */
	ZOOM_MUID		= 14,	/* zoom settings menu */
	FONT_MUID		= 15,	/* font settings menu */
	BROWSE_MUID 	= 16,	/* browse menu */
	CEL_MUID		= 17,	/* cel menu */
	CELPULL_MUID	= 18,	/* cel menu pull */
	ANIPASTE_MUID	= 19,	/* cel paste sub menu */
	FORMAT_MUID 	= 20,	/* fli format menu */
	DRIVER_MUID 	= 21,	/* driver selector menu (multipurpose) */
	TWEENP_MUID 	= 22,	/* Tween pulldown */
	TWEEN_MUID		= 23,	/* Tween panel menu */
	SAVESEG_MUID	= 24,	/* segment save menu */
	BRUSH_MUID		= 25,	/* brush selection menu */
	COMPOS_MUID 	= 26,	/* composite menu */
};

extern Menuhdr quick_menu;
extern Button ink_group_sel;

LONG get_userid(); /* returns serial number of program */

/*** cursors found in cursor.c ******/

#ifndef CURSOR_C
	extern Cursorhdr crosshair16_cursor;
	extern Cursorhdr pentool_cursor;
	extern Cursorhdr zoom_pencel_cursor;
	extern Cursorhdr zoom_cursor;
	extern Cursorhdr menu_cursor;
	extern Cursorhdr hand_cursor;
	extern Cursorhdr fill_cursor;
	extern Cursorhdr pick_cursor;
	extern Cursorhdr plain_ptool_cursor;
#endif

Errcode save_cursor(char *title,Rcel *rc,Short_xy *hot);

Cursorhdr *set_pen_cursor(Cursorhdr *ch);

/* error message and dialog handling and text box functions */

Errcode errline(Errcode err,char *fmt,...);
Errcode softerr(Errcode err,char *fmt,...);

Errcode qchoicef(USHORT *qc_flags,char *fmt,...);
Errcode qchoice(USHORT *qc_flags,char *header, char **choices, int ccount);
int soft_qchoice(USHORT *qc_flags, char *key,...);

/**** zoom functions *****/

void zoom_it(void);
void rect_zoom_it(Coor x,Coor y, Coor w, Coor h);
void upd_zoom_dot(Pixel c, Coor x, Coor y);
void zoom_put_dot(void *r, Pixel c, Coor x, Coor y);
void both_put_dot(void *r, Pixel c, Coor x, Coor y);
void zoom_put_hseg(void *r, void *pixbuf, Coor x, Ucoor y, Ucoor width);
void zoom_put_vseg(void *r,void *pixbuf, Ucoor x,Coor y,Ucoor height);
void zoom_blitrect(void *src, Coor sx, Coor sy,
				   Coor x,Coor y, Coor width, Coor height);
void get_zoomcurs_flixy(Short_xy *xy);

/* grids a point through vs.grid */

void grid_flixy(SHORT *flix, SHORT *fliy);

/* some vpaint specific dotout functions */

void ccolor_dot(SHORT x,SHORT y);
void undo_dot(SHORT x, SHORT y);

/* undo buffer functions */

void undo_rect(Coor x,Coor y,Coor w,Coor h);
void save_undo_rect(Coor x,Coor y,Coor w,Coor h);

/* global undo */

void menu_doundo();
void menu_doredo();

/***** marqi functions ******/

void vinit_marqihdr(Marqihdr *mh,int marqi_it,Boolean bothwins);

/* dotouts */

void undo_marqidot(SHORT x,SHORT y, Marqihdr *mh);
void savedraw_marqidot(SHORT x, SHORT y, Marqihdr *mh);
void restore_marqidot(SHORT x, SHORT y, Marqihdr *mh);

/* rectangles */
void marqi_frame(Marqihdr *mh,SHORT x0,SHORT y0,SHORT x1,SHORT y1);
void marqi_crect(Marqihdr *mh, Cliprect *cr);
void marqi_rect(Marqihdr *mh, Rectangle *r);

Errcode rect_in_place(Rectangle *rect);
Errcode quadpoly_in_place(Short_xy *qpoly);
Errcode rub_rect_in_place(Rectangle *rect);
Errcode r_in_place(SHORT x0,SHORT y0,SHORT x1,SHORT y1);
Errcode clip_move_rect(Rectangle *rect);
Errcode get_rub_clip(Cliprect *clip);
Errcode cut_out_clip(Cliprect *clip);
Errcode get_rub_rect(Rectangle *rect);
Errcode get_srub_rect(Rectangle *rect);
Errcode cut_out_rect(Rectangle *rect);
Errcode gcut_out_rect(Rectangle *rect);

Errcode get_rub_vertex(Short_xy *p0, Short_xy *p1, Short_xy *p2, Pixel color);
Errcode get_rub_line(Short_xy *xys);

void marqi_vector(Marqihdr *mh, Short_xy *pts, int count, int pt_size);
void undo_vector(Marqihdr *mh, Short_xy *pts, int count, int pt_size);

/* standard menu header parts structures and funcs */

typedef struct sg1_data {
	Minitime_data *minidat;
} Sgroup1_data;

extern Button std_head1_sel;
extern Button minipal_sel;
extern Button zpan_cycle_group;

void redraw_head1_ccolor(Button *hanger);
void zpan_ccycle_redraw(Button *hanger);

/**** menu seeme and feelmes ****/

extern void mb_move_quickmenu();
extern void mb_quickmenu_to_bottom();
extern void see_titlebar();
extern void feel_titlebar();
extern void see_cur_ink();

extern void mb_toggle_zclear();
extern void mb_toggle_mask();

/* for use in default input */
Boolean check_pen_abort();	  /* checks for abort outside menus */
Boolean check_esc_abort();	  /* abort menu on ESC */
Boolean check_any_abort();	  /* checks for abort anywhere */
Boolean check_toggle_menu(); /* toggles menu on right click */
Boolean check_toggle_abort(); /* toggles menu on right click, abort on ESC */

/* pencel stuff */

#ifndef PENCEL_C
	#define CELtype void
#else
	#define CELtype Rcel
#endif

Errcode alloc_pencel(Rcel **pcel);
CELtype *clone_pencel(CELtype *toclone);
void copy_pencel(CELtype *s, CELtype *d);
void swap_pencels(CELtype *s, CELtype *d);
void get_penwndo_port(Rectangle *port);

#undef CELtype

/* text stuff */

#define CH_WIDTH 6
#define CH_HEIGHT 8

extern char *clone_string();

extern long get80hz();
extern ULONG pj_clock_1000();
extern long mem_free, largest_frag();

extern void *sort_list();
extern void *remove_el();

#define free_string(pt) pj_free(pt)
#define string_width(s) (strlen(s)*CH_WIDTH)

extern LONG comp_size;






/* Macro related globals */
extern char usemacro,defmacro;	/* Executing macro?  Defining macro? */
extern char inwaittil;	/* Flag to get macros to work during playback... */
extern int recordall;	/* Record even if mouse not moving. */
extern char clickonly;	/* Only record button clicks or key presses */
extern char realtimemac; /* Save time with rest of input... */


struct range
	{
	SHORT min, max, v1, v2;
	};

extern struct range trange;

/* No longer used.	Old version of cluster.  Still part of settings
   file for compatibility.	Blech */

extern char loaded_screen;	/* another flag kludge */

/* An optics move (well except for the path) */
struct ado_setting
	{
	struct ado_setting *next;
	Short_xyz spin_center;
	Short_xyz spin_axis;
	Short_xyz spin_theta;
	SHORT itheta1, itheta2; /* filled in by program */
	Short_xyz size_center;
	SHORT  xp, xq, yp, yq, bp, bq;
	Short_xyz move;
	};

/*** file types for go_files(int type) ***/

enum file_types {
	FTP_FLIC = 0,	/* video */
	FTP_PIC,	/* frame */
	FTP_CEL,	/* cel */
	FTP_PALETTE, /* palette */
	FTP_TEXT,		/* text */
	FTP_FONT,	/* font */
	FTP_POLY,	/* polygon */
	FTP_PATH,		/* path */
	FTP_OPTICS, 	/* optic */
	FTP_SETTINGS, /* settings */
	FTP_MASK,	/* stencil */
	FTP_MACRO,	/* macro */
	FTP_TWEEN,	/* tween */
};
/* names of all our temp files */

extern char tflxname[], disk_tflx_name[], ram_tflx_name[], tmacro_name[],
	alt_name[], cel_name[], screen_name[], text_name[], cclip_name[],
	poly_name[], mask_name[], another_name[], optics_name[], cel_fli_name[],
	tween_name[], poly_name[], default_name[], ppoly_name[], bscreen_name[],
	flxolayname[], poco_source_name[], poco_err_name[], macro_name[],
	tsettings_name[];

extern char *state_temp_files[];
extern char *work_temp_files[];

extern char dirty_file; 	/* need to resave file? */
extern char dirty_frame;	/* need to recompress frame? */
extern long dirty_strokes;	/* # of strokes */

/* RGB values for some of my favorite colors.  Not used much lately. */
extern Rgb3 pure_white, pure_black, pure_red, pure_green, pure_blue;


/* Some variable to maintain start and stop of time range we're
   currently concerned with. */
extern SHORT tr_r1,tr_r2;
extern SHORT tr_rdir;
extern SHORT tr_tix;
extern SHORT tr_frames;

/* What's in the v.cfg file.  The state stuff that doesn't change much
   at all. or is highly machine dependant */

/**** screen init call *****/

Errcode go_screen_menu(Screen_mode *sm);

extern AA_config vconfg;

typedef struct bundle {
	SHORT bun_count;
	UBYTE bundle[256];
} Bundle;

extern UBYTE *cluster_bundle();

#define START_SIXCUBE 0


/* See globals.c initialization of default_vs for more comments on
   this structure.	This is where I try to put all the global
   variables in program VS_MAXCOOR will == a value of relto
   a Vscoor may vary from VS_MAXCOOR*2 to -VS_MAXCOOR*2 */

typedef SHORT Vscoor;	/* a type for scaled coordinates */
#define VS_MAXCOOR (((USHORT)~0)>>2)

SHORT uscale_vscoor(Vscoor vcoor, SHORT relto);
Vscoor scale_vscoor(SHORT coor, SHORT relto);

struct vsettings
	{
	Fat_chunk id;	/* fli file chunk header id */
	SHORT frame_ix; 	/* Current frame */
	SHORT ccolor;		/* Current drawing color */
	SHORT zoomscale;	/* scale factor for zoom window */
	BYTE zoom_open; 	/* the zoom window is open */
	BYTE use_brush; 	/* TRUE == use big brush FALSE == use a dot */
	BYTE dcoor; 		/* display coordinates */
	BYTE fillp; 		/* fill polygons */
	BYTE color2;		/* outline filled polygons? */
	BYTE closed_curve;	/* connect 1'st/last */
	BYTE multi; 		/* do it to many frames? */
	BYTE clear_moveout; /* clear area under move tool */
	BYTE zero_clear;	/* Key color (vs.ink[0]) transparent? */
	BYTE render_under;	/* render cels and overlays only in areas of fli
						 * that are tcolor */
	BYTE render_one_color; /* non tcolor areas of cel to render in ccolor */
	BYTE fit_colors;
	BYTE make_mask;
	BYTE use_mask;
	BYTE pal_fit;		/* fit option on palette menu */
	UBYTE file_type;	/* file_type for files menu */
	UBYTE inks[8];		/* contents of 8 colors in mini palette */
	SHORT ink_id;			/* current Ink type id */
	UBYTE ink_slots[8]; 	/* Inks loaded in option slots */

	SHORT ptool_id; 	 /* current pen tool id */
	UBYTE tool_slots[8]; /* pen tools loaded in option slots */

	Vscoor flicentx, flicenty;		/* fli window center */
	Vscoor quickcentx, quickcenty;	/* quick menu center */
	Vscoor zcentx, zcenty;	/* center for zoomed area */
	Vscoor zwincentx, zwincenty; /* zoom window center */
	Vscoor zwinw, zwinh; /* zoom window size */
	SHORT tint_percent;
	Rectangle twin;  /* text window rectangle */
	SHORT text_yoff;
	LONG tcursor_p;
	SHORT top_tool; /* tool option scroller position */
	SHORT top_ink; /* ink option scroller position */
	SHORT star_points, star_ratio;
	Vscoor gridx, gridy, gridw, gridh; /* grid snap setting */
	UBYTE use_grid;
	UBYTE dthresh;
	SHORT air_speed, air_spread;
	SHORT qdx, qdy; /* quantize grid coordinates */
	Vscoor rgcx, rgcy; /* radial gradient center */
	Vscoor rgr;   /* radial gradient radius */
	Vscoor mkx,mky; /* marked point */
	SHORT transition_frames;
	SHORT start_seg, stop_seg;
	BYTE browse_action;
	BYTE sep_rgb;
	SHORT sep_threshold;
	BYTE ado_tween;
	BYTE ado_ease;
	BYTE ado_ease_out;
	BYTE ado_pong;

	BYTE ado_reverse;
	BYTE ado_complete;
	BYTE ado_source;
	BYTE ado_outline;

	BYTE ado_mode;
	BYTE ado_spin;
	BYTE ado_size;
	BYTE ado_path;

	BYTE ado_mouse;
	BYTE ado_szmouse;
	SHORT ado_turn;
	struct ado_setting move3;
	SHORT sp_tens;
	SHORT sp_cont;
	SHORT sp_bias;
	SHORT time_mode;		/* to frame/segment/all */
	SHORT sep_box;
	SHORT marks[4];
	SHORT starttr[4];
	SHORT stoptr[4];
	SHORT bframe_ix;
	SHORT pal_to;
	BYTE hls;		/* true == hls mode */
	BYTE use_bun;	/* which bundle to use 0 or 1 */
	struct bundle buns[2];
	SHORT cclose;	/* how close a color (%) to go into bundle ? */
	SHORT ctint;		/* WHat percentage to tint in palette remap tint */

	SHORT cdraw_ix;  /* current cycle draw bundle index */
	BYTE cycle_draw;
	BYTE tit_just;
	BYTE tit_scroll;
	BYTE tit_move;
	SHORT pa_tens;
	SHORT pa_cont;
	SHORT pa_bias;
	SHORT pa_closed;
	SHORT cblend;
	SHORT font_height;
	SHORT box_bevel;
	Redo_rec redo;
	SHORT font_type;
	SHORT ped_yoff;
	long ped_cursor_p;

/* cel menu stuff */

	SHORT cur_cel_tool;
	BYTE paste_inc_cel;
	BYTE cm_blue_last;
	BYTE cm_move_to_cursor;
	BYTE cm_streamdraw;
	USHORT rot_grid; /* angular constraint */

	SHORT tween_end;
	SHORT tween_tool;
	SHORT tween_magnet;
	BYTE tween_spline;

	BYTE pen_brush_type;
	SHORT circle_brush_size;
	SHORT square_brush_size;
	SHORT line_brush_size;
	SHORT line_brush_angle;
	SHORT gel_brush_size; /* size of gel tool brush */
	ULONG randseed;

/* composit menu stuff */

	BYTE co_type;
	BYTE co_still;
	BYTE co_cfit;
	BYTE co_reverse;
	BYTE co_matchsize;
	BYTE co_b_first;
	SHORT co_olap_frames;
	SHORT co_venetian_height;
	SHORT co_louver_width;
	SHORT co_boxil_height;
	SHORT co_boxil_width;

/* expand setting */

	Vscoor expand_x, expand_y;

	SHORT font_spacing; /* intercharacter extra spacing */
	SHORT font_leading; /* interline extra spacing */
	SHORT font_unzag;	/* Oversample/antialias outline fonts? */

/* WARNING! do NOT add any pad to this struct. If additional fields are
 * added and the order unchanged the settings loader will fill fields beyond
 * the end of the record with the values in the default_vs in globals.c
 * If pad is saved to the settings file any new fields will be loaded with
 * the pad and the values may be wrong for later additional fields.
 * Just make sure to recompile */

};
typedef struct vsettings Vsettings;

extern Vsettings vs, default_vs; /* see globals.c */


typedef struct vlcb {  /* v local control block not saved !!! */
	Rcel *alt_cel;		/* alternate cel for swapping with fli-cel */

	/* zoom stuff */

	Wndo *zoomwndo; 	/* window attached to zwinmenu if open */
	Rectangle zrect;	/* rectangle containing zoomed area in penwndo */
	SHORT zwinw, zwinh; /* zoom window size */

	/* unscaled menu and window center positions */

	Short_xy zwincent;	/* scaled zoom window center */
	Short_xy flicent;	/* scaled center for fli wndo */
	Short_xy quickcent; /* scaled center for quickmenus */

	Rectangle grid; 	/* grid from vs scaled to current fli-window */
	Short_xy rgc;		/* scaled radial gradient center */
	SHORT rgr;			/* scaled radial gradient radius */

	SHORT flidiag_scale; /* scalar for diag relative items */
	Short_xy scrcent;	/* precalculated center of screen */

	Pentool *ptool; 	/* current pen tool */
	Ink *ink;			/* curent ink tool */
	SHORT hide_brush;	/* hide the brush in the brush cursor */
	VFUNC undoit;		/* current undo function */
	VFUNC redoit;		/* current redo function */
	struct rast_brush *brush;	/* THE drawing brush */
	Rectangle expand_pos;
} Vlcb;

/* globals found in globals.c */

extern Vlcb vl;
extern Flicel *thecel;
extern Rcel *undof;
extern Bitmap *mask_rast;
extern Minitime_data flxtime_data;


extern long pj_ddfree(int device);


/* special pj functions */

Errcode flic_create(char *name,Flifile *flif);

/* leave at end of file */
#endif /* JIMK_H */
