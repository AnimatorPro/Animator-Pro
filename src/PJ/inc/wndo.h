#ifndef WNDO_H
#define WNDO_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef _STDARG_H_INCLUDED
	#include <stdarg.h>
#endif

#ifndef VERTICES_H
	#include "vertices.h"
#endif

#ifndef RECTANG
	#include "rectang.h"
#endif

#ifndef RASTCALL_H
	#include "rastcall.h"
#endif

#ifndef VDEVICE_H
	#include "vdevice.h"
#endif

#ifndef RCEL_H
	#include "rcel.h"
#endif

#ifndef LINKLIST_H
	#include "linklist.h"
#endif

#ifndef INPUT_H
	#include "input.h"
#endif

/********  window structure *********/

#ifdef WNDO_INTERNALS

typedef union cliphead {
	struct {
		SHORT start, next;
		} r; /* r for range */
	union cliphead *nextfree;
} Cliphead;

#define STARTX(ch)  ((((Cliphead *)ch)-1)->r.start)
#define NEXTX(ch)  ((((Cliphead *)ch)-1)->r.next)

void *get_free_clip(Cliphead **freeclips);
void add_free_clip(Cliphead **freeclips, void *clip);

#endif /* WNDO_INTERNALS */

typedef  void (*wndo_redraw_func)(struct wndo *w);

typedef struct wndo {
	RASTHDR_FIELDS;  /* the common raster header for raster library 
					  * note: the fields are a bit different than for a raster
					  * width and height are the width and height of the 
					  * maximum window size. which is the same as the width 
					  * and hight in the "behind" raster wndo->x and wndo->y
					  * are the x and y for the window port onto the screen
					  * and W_xmax and W_ymax (in wndo->body)
					  * are the maximums for this port arranged such that 
					  * w->x, w->y, w->W_xmax, w->W_ymax form a "Cliprect"
					  * structure for the window port. Yet w->width and 
					  * w->height are referenced to the "behind" size 
					  * so that raster clips clip to this size.
					  * All drawing ops done in the window have their origin
					  * 0,0 at the w->behind.x and w->behind.y
					  * the x and y offset for the actual window, not the port
					  * are found in wndo->behind.x wndo->behind.y 
					  * wndo->behind.width and w->behind.height are redundant
					  * with w->width and w->height and are for the sake of the 					  * raster libraries */


/* window body: this MUST be the size of a Rastbody  so that it is compatable
 * with an Rcel see rcel.h and the CEL_FIELDS following will be in the
 * right spot */

	union wndobody {
		Rastbody rb;  /* an Rcel is a raster folowed by CEL_FIELDS */
		struct {
			SHORT xmax, ymax;	/* this is another representation of width and 
							 * height for clipping note this is immediately
							 * after the RastHdr that makes a ClipRect out of 
							 * RastHdr RECT_FIELDS and the wndobody !! */

		#define W_xmax body.wb.xmax
		#define W_ymax body.wb.ymax

			Dlnode node; /* node for installation in window screen window list 
						  * node.next points to window behind
						  * node.prev points to window in front */

		#define W_node body.wb.node

			struct wscreen *screen; /* the screen window is attached to */

		#define W_screen body.wb.screen

		} wb;
	} body;

	CEL_FIELDS; /* colormap etc for cel compatability */

	SHORT userid;   /* user id field for use outside of window system */
	USHORT ioflags; /* ioflags for what io to cause calls to doit */
	int (*doit)(void *wndo); /* returns non 0 if ate initial key input 
						      * function to call when directing input */ 

	/* cliping and backup raster control */

	USHORT flags;
	SHORT W_rastid;	/* raster id for this window rasts[W_rastid] == &behind */
	Raster **rasts;	/* pointers to (behind) rasters for windows
					 * entry [0] is a pointer to the viscel raster 
					 * or the one for wscreen.wndo this is 
					 * "owned by the Wscreen.wndo */
	Raster behind;  /* backup raster containing items behind window */
	UBYTE **ydots;	/* one y dot id list pointer for each x of window */
	union cliphead *free_ydots; /* unused ydotlists */
	SHORT *vchanges; /* one short pointing to next y of vertical change
					  * for each y value */
	SHORT onerast; 	/* for the special libraries is the one raster 
					  * window written to if the library is the onerast 
					  * or one offset rast library set by build_all_clips() */
	SHORT mc_csetid; /* color set id for last redraw */
	wndo_redraw_func redraw; 		/* function to redraw the menucolor
									 * items in a window */
	Cursorhdr *cursor;
	VFUNC procmouse; 	/* function to process mouse input within c_input() */
	void *doit_data;	/* data pointer for doit function */
	LONG for_the_futcha[1];
} Wndo;


#define WNDO_MUCOLORS  	0x0001 /* this window wants to use the menucolors */
#define WNDO_HIDDEN		0x0002 /* window is in "hidden state" */
#define WNDO_BACKDROP	0x0004 /* window is a backdrop window that does not
								* save screen behind it and may not be opened
								* in from of a non backdrop window */

/* menu group node used by menu system */

typedef struct mugroup {  /* used only by menu system but part of screen */
	Dlnode snode;         /* node for stack */
	Dlheader menuhdrs;    /* list of menus in group */
	struct wscreen *screen; /* screen group attached to */
	LONG retcode;    /* return code from last menu close or group close */
	SHORT num_menus;  	/* number of menus in this group */
	SHORT non_hidden; 	/* number of non hidden menus in this group */
	USHORT flags; 		/* allways */
	SHORT hmpcnt;	    /* count for hide stack */
	struct button *tabnext;	/* used for entry field tabbing */
	LONG unused[3];
} Mugroup;

/* menu group flags values */

#define MUG_REQUESTOR   0x0001 /* this is a requestor group */

/* structure for menu scaling. For now our menus are based on a root scale
 * of 320 X 200 this may be changed later */

typedef struct rscale {
	SHORT xscalep, xscaleq;		/* Scale factors for x. Scale = p/q */
	SHORT yscalep, yscaleq;
} Rscale;

/* The big screen to put windows on that can itself be used as a window */

typedef struct wscreen { 
	Wndo wndo; /* the actual full screen window so the first field 
				* so screen itself is a clipped window raster */	

	Vdevice *dispvd; /* display raster driver */
	Vdevice *wndovd; /* window backup raster driver */

	/* two cels !! these are swapped (not supported) Nice to fantasize
	 * in the double buffered mode */

	Rcel *viscel;  	/* the visible cel */
	Rcel *offcel;   /* the off screen */

	USHORT max_wins;	/* maxumum number of windows this screen can have */
	USHORT num_wins;	/* current number of open windows */
	Dlheader wilist;	/* window list head is top window tail bottom */

	struct vfont *mufont;	   	/* default font for menus and textboxes */

	USHORT flags;               /* allways */

/* system coopted color control stuff for maintenence of visible colors */

#define FIRST_MUCOLOR (251)         /* first one we'll use */
#define NUM_MUCOLORS	5			/* number of managed menu colors */

	UBYTE mc_alt;                   /* cmap has been altered to accomodate
									 * menucolors one bit for each mucolor
									 * starting with bit 0 */
	UBYTE mc_lastalt;				/* last altered colors */
	Pixel mc_colors[NUM_MUCOLORS];	/* menu colors that are to be visible */ 
	Rgb3 mc_lastrgbs[NUM_MUCOLORS];	/* the colors found for last mc_colors */
	Rgb3 *mc_ideals;		  		/* colors to seek for menu mc_colors */
	SHORT mc_csetid;				/* id number for current color set */
	SHORT refresh_disables;         /* for enable - disable auto refresh */
	struct wndo *wait_box;          /* "please wait" window if present */
	UBYTE bbevel;					/* bevel for rounded buttons */
	UBYTE is_hires;					/* Width bigger than 500? */
	Rectangle last_req_pos;			/* last requestor position */
	SHORT unused0[1];
	Cursorhdr *menu_cursor;			/* default cursor for opening menus */
	Cursorhdr *cursor;				/* default cursor for opening windows */

/* stuff added for exclusive use by menu system ignored by window library */

#ifdef WNDO_INTERNALS 
	#define WS_FIRST_MENUFIELD group0 
#endif /* WNDO_INTERNALS */

	Mugroup group0;			    	/* The default group not on stack */
	Dlheader gstack;				/* a stack of groups, the head is the
									 * group in the current input loop */
	SHORT glevel;					/* number of groups in stack 
									 * bottom level is 1 */
	Rscale menu_scale;				/* scaleing for scaled menus */

} Wscreen;


/* wscreen flags values same as window flags */

#define WS_MUCOLORS 		WNDO_MUCOLORS 	/* enable use of menucolors */ 

/* unique flags to screen */

#define WS_REFRESH_SET	0x2000  /* a request to refresh the windows has been 
								 * set */
#define WS_NOMENUS		0x4000  /* in init flags don't allocate menu part 
								   * starting with WS_FIRST_MENUFIELD 
								 * in screen flags "its not a menu screen" */ 
#define WS_MUCOLORS_UP	0x8000  /* menu colors are currently needed by 
								 * windows */


/* menu color numbers */

#define MC_BLACK 	0
#define MC_GREY 	1
#define MC_WHITE 	2
#define MC_BRIGHT 	3
#define MC_RED 		4

/* these can be used as field names on a wscreen */

#define SBLACK mc_colors[MC_BLACK]
#define SGREY mc_colors[MC_GREY]
#define SWHITE mc_colors[MC_WHITE]
#define SBRIGHT mc_colors[MC_BRIGHT]
#define SRED mc_colors[MC_RED]

/* input structure to open_wndo() */

typedef struct wndoinit {
	RECT_FIELDS;     /* x,y,widht,height */
	SHORT maxw, maxh; /* maximum width and height f bigger than above wil use
					   * otherwise will take width and height as max */
	void *screen;  /* screen to attach window to */
	Wndo *over;    /* the window to install this one under when opened 
					* NULL or a window not opened on the screen 
					* is to install window on top of the rest */
	USHORT flags;
	SHORT extrabuf;	/* extra data buffer to allocate and append to window
					 * when opening for user data area it will start at
					 * window + 1 */
	Cursorhdr *cursor; /* cursor for window */
} WndoInit;

/* NOTE: flags WNDO_MUCOLORS and WNDO_HIDDEN are installed in the
 * wi->flags variable to request menucolors for window and open it in the 
 * hidden state.
 * WNDO_BACKDROP is to open the window as a backdrop window in which case
 * over will be ignored */
 
 /* flags unique to wndoinit */

#define WNDO_NOCLEAR	0x8000	/* do not clear window when opening */


#define WNDO_MINHEIGHT	2  /* smallest we allow */
#define WNDO_MINWIDTH	2  /* smallest we allow */

#define MAX_WNDOS	20 /* the largest number of windows one can open 
						* for a screen including the screen window */

typedef struct wscrinit {
	USHORT flags;          /* we of course need one of theese in everything */
	USHORT max_wins;       /* maximum number of windows to allow on screen
							* including the screen window (backdrops are 
							* subsumed by the screen wndo and are only limited
							* by available memory) at least 1 */ 
	Rcel *cel_a;			/* cel for initial on screen cel */
	Rcel *cel_b;			/* cel for off screen cel */

	Vdevice * disp_driver;   /* driver used for display rasters */
	Vdevice * wndo_driver;   /* driver used for window backup rasters if left 
						    * NULL will use ram driver by default */
	Cursorhdr *cursor;     /* initial cursor for screen */
} WscrInit;

/* flags for WscrInit */


/* screen things */

Errcode open_wscreen(Wscreen **ps, WscrInit *si);
void close_wscreen(Wscreen *s);
void set_input_screen(Wscreen *s);
void set_ext_cel(Wscreen *s,Rcel **ext_cel);
Boolean find_mucolors(Wscreen *ws);

/* textbox "takeover" requestor things */

#define TBOX_MAXCHOICES	5
Errcode tboxf(Wscreen *s,char *fmt,va_list args);
Errcode tboxf_choice(Wscreen *s,char *formats,char *text,va_list args,
					 char **choices, char *extratext);
Errcode va_wait_wndo(Wscreen *screen, char *wait_str, 
					 char *formats, char *fmt, va_list args);
void cleanup_wait_wndo(Wscreen *s);


/* layer window things */

Errcode open_wndo(Wndo **pw, WndoInit *wi);
void close_wndo(Wndo *w);
void get_wndo_oset(Wndo *w, Short_xy *oset);
Boolean reposit_wndo(Wndo *w,Rectangle *newpos,Short_xy *oset);
void move_wndo(Wndo *w, SHORT dx, SHORT dy);
Boolean ptin_wndo(Wndo *w,SHORT x,SHORT y);
Boolean wndo_dot_visible(Wndo *w,Coor x,Coor y);
Boolean curson_wndo(Wndo *w);
Boolean mouseon_wndo(Wndo *w);
void redraw_wndo(Wndo *w);
void init_wrefresh(Wscreen *s); 

/* set, save and restore icb input state for a window */

void load_wndo_mouset(Wndo *w); 

typedef struct Wiostate {
	Mouset mset;
	VFUNC procmouse;
	Cursorhdr *cursor;
	Wndo *iowndo;
} Wiostate;

extern void save_wiostate(Wiostate *ios);
extern void rest_wiostate(Wiostate *ios);

/* flags used in icb.wflags */

#define IWF_TBOXES_OK	0x0001   /* text boxes allowed */

void get_requestor_position(Wscreen *ws, SHORT width, SHORT height, 
						    Rectangle *pos);

#ifdef WNDO_INTERNALS

#define SCREEN_RASTID 0
#define NULL_RASTID   1
#define FIRST_OPENRASTID 2

struct rastlib;

extern struct rastlib *get_window_lib(void); /* multi clip lib */
extern struct rastlib *get_wndo_r1lib(void); /* one raster lib */
extern struct rastlib *get_wndo_r1oslib(void); /* one raster offset lib */

void build_all_clips(Wscreen *ws,USHORT full_update);

#endif /* WNDO_INTERNALS */

#endif /* WNDO_H */
