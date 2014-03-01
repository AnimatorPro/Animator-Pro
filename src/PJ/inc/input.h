#ifndef INPUT_H
#define INPUT_H


/* non ascii key definitions */
#define PAGEUP  0x4900
#define PAGEDN  0x5100
#define ENDKEY  0x4f00
#define HOMEKEY 0x4700
#define DELKEY  0x5300
#define INSERTKEY 0x5200
#define LARROW	0x4b00
#define RARROW	0x4d00
#define UARROW	0x4800
#define DARROW	0x5000
#define FKEY1	0x3b00
#define FKEY2   0x3c00
#define FKEY3	0x3d00
#define FKEY4	0x3e00
#define FKEY5	0x3f00
#define FKEY6	0x4000
#define FKEY7	0x4100
#define FKEY8	0x4200
#define FKEY9	0x4300
#define FKEY10	0x4400
#define FKEY11	0x4500
#define FKEY12	0x4600
#define CTRL_C  0x2e03

/* ascii but non printable defs */
#define ESCKEY 0x1b

#ifndef REXLIB_CODE 

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef LINKLIST_H
	#include "linklist.h"
#endif

#ifndef VERTICES_H
	#include "vertices.h"
#endif

#ifndef RECTANG_H
	#include "rectang.h"
#endif

/* size of histerisis buffer (used by gel and drizzle mostly) */
#define HSZ 4	


/* the global input control block this should be considered a read
 * only structure by code outside of the input directory or the 
 * macro code */

struct cursorhdr;

typedef void (*cursorhdr_func)(struct cursorhdr *ch,...); 
 typedef struct cursorhdr {
	cursorhdr_func showit; /* display mouse cursor */
	cursorhdr_func hideit; /* un-display mouse cursor */
 	cursorhdr_func moveit; /* move displayed cursor */
 } Cursorhdr;


extern Cursorhdr null_cursor; /* a do nothing invisible cursor */
/* generic move cursor function calls hideit then showit */
extern void gen_move_cursor(Cursorhdr *ch); 
extern void set_cursor(Cursorhdr *cd);

typedef struct mouset
{
	SHORT wndoid;   /* for use by window system */
	Short_xy oset;	/* mouse offsets used to get mx and my */
	UBYTE on;		/* user wants mouse on */
	UBYTE unused0;
	LONG unused1[4];		
} Mouset;


#ifdef INPUT_INTERNALS /* stuff used only in macro and input code */

#ifndef PTRMACRO_H
	#include "ptrmacro.h"
#endif

Boolean _poll_input(Boolean do_cursor);

#define WANTDRAWCURS (icb.mcurs_up++ == 0 && icb.mset.on)
#define DRAWCURSOR() {(*(icb.curs->showit))(icb.curs);}
#define WANTUNDRAWCURS ((--icb.mcurs_up) == 0 && icb.mset.on)
#define UNDRAWCURSOR() {(*(icb.curs->hideit))(icb.curs);} 

#define SHOWCURSOR() {if(WANTDRAWCURS) DRAWCURSOR();}
#define HIDECURSOR() {if(WANTUNDRAWCURS) UNDRAWCURSOR();}

#define MOVECURSOR() {\
	if(icb.mset.on && icb.mcurs_up > 0)\
		(*(icb.curs->moveit))(icb.curs);}

#define SET_BUPBITS(state) \
	((state)|=(~((state)<<BUPBIT1))&((MBPEN|MBRIGHT)<<BUPBIT1))

struct icb_coords { /* needs to be same as field order in global icb */
	SHORT mx, my;
	SHORT cx, cy;
	SHORT sx, sy;
	SHORT pressure;
};

#define ICB_COPYTO_LAST() \
 *((struct icb_coords *)&(icb.lastmx))=*((struct icb_coords *)&(icb.mx))

#define ICB_COPYFROM_LAST() \
 *((struct icb_coords *)&(icb.mx))=*((struct icb_coords *)&(icb.lastmx))

#define CHECK_POP_ICB(pushed) { if(icb.push != pushed) _pop_icb(pushed); }

#endif /* INPUT_INTERNALS */

/*****************  Global Input Control block structure ******************/

/* NOTE that all SHORT x,y pairs are in the order x-y so that they may be 
 * as a Short_xy structure */

/* the two field blocks "currents and lasts" must be the same size and in the
   same order so a block copy may be done between them the structure
   "icb_coords" above needs to be maintained to match the size and order
   of these fields */

typedef struct global_icb {

/* ------Currents------*/
	SHORT mx, my;		/* offset mouse x and y for current mouse 
						 * settings ie: offset, gridding etc. */
	SHORT cx, cy;		/* cursor x and cursor y */
	SHORT sx, sy;		/* unaltered screen mouse x and y */
	SHORT pressure;		/* Pressure scaled from 0-255 */
/* -------lasts--------*/
	SHORT lastmx, lastmy; /* last menu/window x and menu/window y */
	SHORT lastcx, lastcy; /* last cursorx and cursory */
	SHORT lastsx, lastsy; /* last screenx and screeny */
	SHORT last_pressure;  /* last pressure scaled from 0-255 */
/*---------------------*/

	ULONG state;    	/* button down states and input recieved flags */
	ULONG ostate;		/* previous (old) button down states and flags */
	ULONG xorstate;		/* ostate^state indicates state transits */
	ULONG hitstate;		/* xorstate & state indicates "transit to on" */
	SHORT inkey;        /* the current input key */
	Cursorhdr *curs;    /* current cursor */
	VFUNC procmouse;	/* function to process mouse inside _poll_input() */

	struct wscreen *input_screen;	/* set by window system */
	struct wndo *iowndo;			/* set by window system */
	Mouset mset; 		/* current window mouse settings */
	USHORT wflags;		/* window system flags used by window system */

#ifdef INPUT_INTERNALS

/* private fields not accessed outside of input or macro */

	UBYTE reuse;		/* boolean flag reuse input */
	BYTE  mcurs_up;     /* cursor currently displayed on screen 
						 * it is incremented for each showcursor and
						 * decremented for each hide */

	/* macro recording control */

	ULONG recflags;		/* allow macro record of this in non-realtime mode */

	ULONG waithit; 		/* hitstate flags being checked in _poll_input() */  
	ULONG input_eaten;  /* input eaten by a waitask or inside _poll_input() 
						 * anything set here will be recorded in macro and
						 * deleted from _poll_input()'s output */

#define ICB_PUSH_SIZE  OFFSET(Global_icb,push)

	void *push; /* used for recursive input state saving */

	/**** items beyond here not part of "pushed" or "poped" iostate ****/


	BYTE macro_mode;      /* MAKE_MACRO, USE_MACRO | MACR0_OK */
	BYTE macro_clocked;   /* true if macro clocked in play 
						   * or to record all mouse moves in record */

	Cliprect sclip;     /* boundary to clip to for current screen */
	LONG p[3],q[3];	   /* used for scaling input devices */
	struct idriver *idriver; /* pointer to possibly loaded input driver */
	UBYTE reads_pressure;	/* the pointing device reads pressure */
	USHORT clocks_per_field;

	/* these items are used so that macro and hot key functions are not
	 * forced to link with input if they arn't needed. set_hotkey_func()
	 * will set do_hot_key() and the macro system will install the macro 
	 * function pointers */

	Boolean (*do_hot_key)(struct global_icb *gicb);

#endif /* INPUT_INTERNALS */

} Global_icb;

typedef struct icb_savebuf Icb_savebuf;

#ifdef INPUT_INTERNALS

struct icb_savebuf {
	UBYTE buf[ICB_PUSH_SIZE];
};

#undef ICB_PUSH_SIZE

/* macro_modes values recording or playing if > MACRO_OK */

#define MACRO_OK	0x10 	/* if this flag is removed it suspends io use */
#define MAKE_MACRO	(1|MACRO_OK) 
#define USE_MACRO   (2|MACRO_OK) 

#endif /* INPUT_INTERNALS */

#define PRESSURE_MAX 0xFFU	/* highest pressure value possible */ 

/******************** button down state flags ********************/
/* Some keys and mouse buttons and flags for other input present */

#define MBPEN		0x01U  /* pen or left mouse button is down */
#define MBRIGHT		0x02U  /* right mouse button is down */
#define MBMIDDLE   	0x04U  /* middle mouse button is down */

/* 0x08U unused */
/* 0x10U unused */
/* 0x20U unused */

#define BDOWNBITS 0x3F /* all above */

#define KEYHIT      0x40U  /* key(s) are in keyboard buffer */

#define MACRO_REC   0x80U  /* this is used in macro system for 
					  		* extended record types needs to be last bit
							* of first byte (in address order) of first
							* short word (low 16 bits) of the flags */

/* start of "second" byte */

#define BUPBIT1	8   /* first bit of dual bit button flags */
#define MBPUP		0x0100U /* pen is up */
#define MBRUP		0x0200U /* right is up */
#define MBMUP		0x0400U /* middle is up */

/* 0x0800U unused */
/* 0x1000U unused */
/* 0x2000U unused */

#define MMOVE		0x4000U  /* screen mouse has moved in x or y */
#define HISTATE		0x8000U  /* the low priority state flags in the
							  * next 16 bits are changed */

/* lower priority keyboard button states in high bits */

#define KBRSHIFT       0x10000U  /* right shift key */
#define KBLSHIFT       0x20000U  /* left shift key */
#define KBRCTRL        0x40000U  /* right control key (for now both same ) */
#define KBRALT         0x80000U  /* right alt key (for now both are this) */
#define KBSCRLOCK	  0x100000U  /* scroll lock key */
#define KBNUMLOCK	  0x200000U  /* num lock key */
#define KBCAPLOCK	  0x400000U  /* caps lock key */

/* note these two flags are never hit at present, keyboard doesn't 
 * differentiate */

#define KBLCTRL       0x800000U /* left control key */
#define KBLALT       0x1000000U /* left alt key */

/* some convenient combos */
#define KBSHIFT	(KBRSHIFT|KBLSHIFT)
#define KBCTRL	(KBRCTRL|KBLCTRL)
#define KBALT	(KBRALT|KBLALT)

#define ICB_TIMEOUT	0x10000000U  /* anim wait input timed out or first call 
								 * for use inside anim funcs only */

#define HI_BSTATES  0x01FF0000U  /* low priority upper 16 bit states that will
							      * set HISTATE in low word on transit */ 

/* all toggling button up or down state flags */
#define ALL_BSTATES ~(MACRO_REC|MMOVE|KEYHIT|HISTATE) 

/***** flags to be passed into wait_input(ULONG flags) *****/

#define ANY_INPUT ((ULONG)~0)
#define ANY_CLICK ((ULONG)KEYHIT|MBPEN|MBRIGHT)

/* Any ored combo of the button state flags may be passed into wait_inpt()
 * and it will wait until any of the input flags has a transition ie: just
 * up or just down hit state.
 * 
 * The other flags MMOVE and KEYHIT may be ored in also and 
 * will cause wait_input() to return if they are turned on or were on when
 * wait_input() was called.
 * 
 * To wait on a pen up/down or a mousemove call wait_input(MMOVE|MBPEN); */


/***** Macros for tests on the current button and key states *****/

/* the JSTHIT() macro may test for any one of the flags in bflags 
 * ie: if(JSTHIT(MBPEN|MBRIGHT|KEYHIT|MMOVE)) == if pen button just went down
 * or right button just went down or a key was hit and is waiting in icb.inkey.
 * or the mouse has moved since last input read.
 */

#define JSTHIT(bflags) (icb.hitstate&(bflags))
#define ISINPUT(bflags) (icb.hitstate&(bflags))
#define ISDOWN(bflags) (icb.state&(bflags))
#define ALLDOWN(bflags) ((icb.state&(bflags))==(bflags))


/* exported references */

extern Global_icb icb;  /* see input.c */

SHORT dos_wait_key(void); /* very low level key wait, not recorded in macros
						   * flushes buffer first, for use in low level error 
						   * requestors */

Errcode init_idriver(char *name, UBYTE *modes, SHORT comm_port);
void cleanup_idriver(void);
Errcode reset_input(void);

void reuse_input(void);

Boolean check_input(ULONG checkflags);
void wait_input(ULONG waitflags);
void mac_wait_input(ULONG waitflags,ULONG recflags);
Errcode vsync_wait_input(ULONG waitflags, SHORT fields);
Errcode mac_vsync_wait_input(ULONG waitflags, ULONG recflags, SHORT fields);
Errcode timed_wait_input(ULONG waitflags, ULONG timeout_millis);

void wait_a_jiffy(int j);
void wait_any_input(void);
void wait_click(void); /* wait_input(ANY_CLICK); synonym */
void wait_penup(void);
void wait_mbup(ULONG flags);

/* repeat function while pen is down */
extern void repeat_on_pdn(void (*v)(void *data), void *data);

/* sets preemptive function for processing keys */
FUNC set_hotkey_func(Boolean (*do_hot_key)(Global_icb *gicb));

/***** icb state saving and restoring ****/

void save_icb_state(Icb_savebuf *save_area);
void restore_icb_state(Icb_savebuf *saved);
Icb_savebuf *check_push_icb();

#ifdef INPUT_INTERNALS
	#define FDAT void *funcdata
#else
	#define FDAT ...
#endif

int anim_wait_input(ULONG waitflags, ULONG funcflags, 
					int maxfields, FUNC func, FDAT);

#undef FDAT

void get_mouset(Mouset *mset);
void load_mouset(Mouset *mset);
void set_procmouse(VFUNC procmouse);
void set_mouse_oset(SHORT mosetx, SHORT mosety);
void reset_icb(void);
Boolean hide_mouse(void);
Boolean show_mouse(void);
void display_cursor();
void undisplay_cursor();
SHORT toupper_inkey(); /* returns the icb.inkey with the ascii part
						* toupper()ed */

/******* macro synchronized user abort polling stuff ******/

typedef UBYTE Abortbuf[12];  /* see macro.c */
Errcode poll_abort(void);
void pstart_abort_atom(Abortbuf *ab);
void start_abort_atom(void);
Errcode end_abort_atom(void);
Errcode errend_abort_atom(Errcode err);

/****** input polling "wait task" items *******/

typedef struct waitask {
	Dlnode node;
	int (*doit)(struct waitask *wt); /* if returns !0 task removed 0 
									  * it leaves it in list */
	void *data;
	SHORT flags;
} Waitask;

#define WT_KILLCURSOR	0x0001
#define WT_ATTACHED		0x0002

#define WT_ISATTACHED(wt) ((wt)->flags & WT_ATTACHED)

void init_waitask(Waitask *wt, FUNC func, void *data, USHORT flags);
void add_waitask(Waitask *wt);
void rem_waitask(Waitask *wt);
void disable_wtasks();
void enable_wtasks();
int wdisable_dofunc(FUNC func, void *dat);

#endif /* REXLIB_CODE */

#endif /* INPUT_H */
