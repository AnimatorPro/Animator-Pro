/* input.c  -  This is the messy mouse section.  Responsible for updating
   the values in the Global_icb icb structure 
   are updated every time _poll_input is called.  Macros are taken
   care of here by calling the appropriate routines in macro.c.
   */

#define INPUT_INTERNALS

#include "jimk.h"
#include "errcodes.h"
#include "idriver.h"
#include "imath.h"
#include "input.h"
#include "rastcall.h"

Global_icb icb; 

void wait_sync(void)
/* wait for sync with vertical blanking of current input screen */
{
	if(icb.input_screen)
		pj_wait_rast_vsync(icb.input_screen);
	else
		pj_wait_vsync();
}
static void mwaits(void)
{
#ifdef INPUT_MACROS
	if((icb.macro_mode != USE_MACRO) || icb.macro_clocked) 
		wait_sync();
#else

	wait_sync();

#endif /* INPUT_MACROS */
}
void wait_millis(int millis)
/* pause for a couple of milliseconds */
{
ULONG l;

	l = pj_clock_1000()+millis;
	for (;;)
	{
		if (pj_clock_1000() >= l)
			break;
		mwaits();
	}
}

void wait_a_jiffy(int j)
/* pause a couple of frame times */
{
	wait_millis(pj_uscale_by(j,1000,70));
}

Boolean is_pressure(void)
{
	return(icb.reads_pressure);
}
void cleanup_idriver(void)
{
	close_idriver(&icb.idriver);
}
Errcode init_idriver(char *name, UBYTE *modes, SHORT comm_port)
{
	return(load_idriver(&icb.idriver,name,modes,comm_port));
}
Errcode reset_input(void)

/* This must be called befor input is used and 
 * and also to reset input and clear buffers when we have opened 
 * or changed the input screen dimensions */
{
long ap, aq, xclip, yclip;
long idrw, idrh;
Raster *screen = (Raster *)(icb.input_screen);
#define SCREEN_AX	320
#define SCREEN_AY	200

	set_cursor(NULL);

	icb.recflags = ANY_INPUT;

	if(!(icb.idriver) || !(screen))
		return(Err_bad_input);

	icb.reads_pressure = (icb.idriver->channel_count >= 3 
							&& (icb.idriver->flags[2]&PRESSURE));

	if(icb.reads_pressure)
	{
		icb.p[2] = PRESSURE_MAX+1;
		icb.q[2] = icb.idriver->max[2] - icb.idriver->min[2] + 1;
	}

	idrw = icb.idriver->max[0] - icb.idriver->min[0];
	idrh = icb.idriver->max[1] - icb.idriver->min[1];

	icb.p[0] = screen->width;
	icb.q[0] = idrw + 1;
	icb.p[1] = screen->height;
	icb.q[1] = idrh + 1;
	ap = icb.idriver->aspect[0]*SCREEN_AY;
	aq = icb.idriver->aspect[1]*SCREEN_AX;

	if (ap < aq)  /* Y dimension of input device too long */
	{
		icb.p[1] *= aq;
		icb.q[1] *= ap;
	}
	else if (ap > aq)	/* X dimension of input device too long */
	{
		icb.p[0] *= ap;
		icb.q[0] *= aq;
	}
	/* else if aspect ratio is the same do nothing. */


	if((idrw*icb.p[0])/icb.q[0] >= screen->width)
		xclip = icb.idriver->min[0] + ((screen->width*icb.q[0])/icb.p[0]);
	else
		xclip = -1;

	(*icb.idriver->lib->setclip)(icb.idriver,0,xclip);

	if((idrh*icb.p[1])/icb.q[1] > screen->height)
		yclip = icb.idriver->min[1] + ((screen->height*icb.q[1])/icb.p[1]);
	else
		yclip = -1;

	(*icb.idriver->lib->setclip)(icb.idriver,1,yclip);

	check_input(ANY_INPUT); 
	ICB_COPYTO_LAST(); /* set lasts to current values */

	while(icb.hitstate) /* flush any buffers */
		check_input(ANY_INPUT);

	icb.clocks_per_field = 15;

	return(0);
}

/***** function to set "hot" key function returns pointer to old hot key
 	   function *****/

FUNC set_hotkey_func(Boolean (*do_hot_key)(Global_icb *gicb))
{
FUNC ohot;

	ohot = icb.do_hot_key;
	icb.do_hot_key = do_hot_key;
	return(ohot);
}

/***** functions to load and alter mouse settings control in the icb *******/

static void do_nocursor(Cursorhdr *ch)
{
	(void)ch;
}

static Cursorhdr null_cursor = {
	do_nocursor,
	do_nocursor,
	do_nocursor,
};

void gen_move_cursor(Cursorhdr *ch)
{
	(void)ch;

	(*(icb.curs->hideit))(icb.curs);
	(*(icb.curs->showit))(icb.curs);
}

void set_cursor(Cursorhdr *cd)
{
	if(!cd || !cd->showit)
		cd = &null_cursor;
	if(!cd->hideit)
		cd->hideit = (cursorhdr_func)do_nocursor;

	if(cd->moveit == NULL)
		cd->moveit = (cursorhdr_func)gen_move_cursor;

	if(icb.mset.on 
		&& cd != icb.curs 
		&& icb.mcurs_up > 0)
	{
		UNDRAWCURSOR(); 
		icb.curs = cd;
		DRAWCURSOR();
	}
	else
		icb.curs = cd;
}

void display_cursor(void)
/* this will actually draw the cursor on the screen if it is not and will
 * increment the cursor on count will actually make it visible only if the
 * cursor is in the "on" state */
{
	SHOWCURSOR();
}
void undisplay_cursor(void)

/* inverse of display cursor */
{
	HIDECURSOR();
}

Boolean hide_mouse(void)
{
Boolean was_on = icb.mset.on;
	icb.mset.on = 0;
	if(icb.mcurs_up > 0 && was_on)
		UNDRAWCURSOR();
	return(was_on);
}
Boolean show_mouse(void)
{
Boolean was_on = icb.mset.on;
	icb.mset.on = 1;
	if(icb.mcurs_up > 0 && !was_on)
		DRAWCURSOR();
	return(was_on);
}
void get_menucursorxy(void)

/* apply Mouset settings to sx and sy to get cursor position and
 * window local gridded values for mx, my, cx, and cy */
{
	icb.cx = icb.mx = icb.sx;
	icb.cy = icb.my = icb.sy;
	icb.mx -= icb.mset.oset.x;
	icb.my -= icb.mset.oset.y;
	if(icb.procmouse)
		(*icb.procmouse)();
}
/***** functions for settings changes *****/

void reset_icb(void)

/* called after settings change to reset calculated values */
{
SHORT curx, cury;

	curx = icb.sx;
	cury = icb.sy;
	icb.sx = icb.lastsx;
	icb.sy = icb.lastsy;
	icb.pressure = icb.last_pressure;
	get_menucursorxy(); /* recalc previous values as current */
	ICB_COPYTO_LAST();
	icb.sx = curx;
	icb.sy = cury;
	get_menucursorxy(); /* recalc current values */
}
void set_mouse_oset(SHORT osetx, SHORT osety)
{
	icb.mset.oset.x = osetx;
	icb.mset.oset.y = osety;
	reset_icb();
}
void set_procmouse(VFUNC procmouse)
{
	icb.procmouse = procmouse;
	reset_icb();
}
void get_mouset(Mouset *mset)
{
	*mset = icb.mset;
}
void load_mouset(Mouset *mset)
{
	if(mset->on)
		show_mouse();
	else
		hide_mouse();
	icb.mset.oset = mset->oset;
	reset_icb();
}

/******************************************************************/
void reuse_input(void)
{
	icb.reuse = 1;
}
/***** _poll_input() recursion mechanism *******************************
 * if we are a level down we "push" the icb into the push pointer set
 * to the save area of the previous levels stack. and set the new push
 * pointer to the current stack frame. if the 
 * icb was "pushed" by a recursive _poll_input() the stack frame of the
 * current "push" pointer will not be the current frame but of the 
 * recursion that pushed it so if not same we pop! this will avoid
 * redundant push-pops unless the recursion level is actually changing 
 * the struct icb_savebuf is the size of the data which is saved only
 * the data which is not pushed is the leading sizeof(struct icb_savebuf) 
 * part of the icb */

void save_icb_state(Icb_savebuf *save_area)
{
	*save_area = *((Icb_savebuf *)&icb); /* save input state */
}
void restore_icb_state(Icb_savebuf *saved)
{
	if(icb.mcurs_up > 0 && icb.mset.on)
		UNDRAWCURSOR();
	*((Icb_savebuf *)&icb) = *saved;
	if(icb.mcurs_up > 0 && icb.mset.on)
		DRAWCURSOR();
}
Icb_savebuf *check_push_icb(void)
/* checks if we are a level down and "pushes" the icb state guarantees
 * that new state is with cursor count off in "virgin" condition 
 * returns pointer to buffer if it did push the global_icb 
 * buffer only valid within stack frame of _poll_input() one level up */
{
Icb_savebuf *pushed;

	if(icb.push) 					 /* if we are entering one level down */
	{
		pushed = icb.push;
		if(icb.mcurs_up > 0 && icb.mset.on)
			UNDRAWCURSOR();
		save_icb_state(icb.push);
		icb.mcurs_up = 0;
		icb.push = NULL;
		return(pushed);
	}
	return(NULL);
}
void _pop_icb(Icb_savebuf *pushed)
{
  	wait_mbup(MBPEN|MBRIGHT);
	restore_icb_state(pushed);
    icb.push = pushed;
}
Boolean _poll_input(Boolean do_cursor)
{
Boolean ret;
Icb_savebuf recurs;

	icb.push = &recurs; /* set to current stack frame */

#ifdef TESTING
	check_a_cookie(); /* will scan for cookies with bad data */
#endif

	if(icb.reuse)
	{
		icb.reuse = 0;
		if(icb.hitstate & icb.waithit)
			goto done_true;
		goto done_false;
	}

	/* move current state to last state */

	icb.ostate = icb.state & ALL_BSTATES;
	ICB_COPYTO_LAST();

#ifdef INPUT_MACROS
	if(icb.macro_mode == USE_MACRO)
	{
		ret = get_macro_input();
		CHECK_POP_ICB(&recurs);
		if(ret >= Success)
			goto got_input; /* if not macro closed or aborted or error */
	}
#endif /* INPUT_MACROS */

	(*icb.idriver->lib->input)(icb.idriver);
	idr_clip((Idriver *)icb.idriver,0,1);

	/* only look at 2 buttons */
	icb.state = icb.idriver->buttons & (MBPEN|MBRIGHT);

	icb.sx = ((icb.idriver->pos[0]-icb.idriver->min[0])*icb.p[0])/icb.q[0];
	icb.sy = ((icb.idriver->pos[1]-icb.idriver->min[1])*icb.p[1])/icb.q[1];

	if(icb.reads_pressure)
	{
		icb.pressure = ((icb.idriver->pos[2]-icb.idriver->min[2])
							*icb.p[2])/icb.q[2];
	}
	else
		icb.pressure = PRESSURE_MAX;

	/* note that we have intel word in long word order here to 
	   shift caps bits to upper 16 */

	((USHORT *)&icb.state)[1] |= 
		(((KBRSHIFT|KBLSHIFT|KBRCTRL|KBRALT
			|KBSCRLOCK|KBNUMLOCK|KBCAPLOCK)>>16) & dos_key_shift());

	if(!icb.idriver->does_keys)
	{
		if(pj_key_is())
		{
			icb.inkey = pj_key_in();
			goto got_inkey;
		}
		else
			icb.inkey = 0;
	}
	else if((icb.inkey = icb.idriver->key_code) != 0)
	{
got_inkey:

		icb.state |= KEYHIT;
	}


got_input:

	if (icb.sx != icb.lastsx || icb.sy != icb.lastsy)
	{
		icb.state |= MMOVE;

#ifdef INPUT_MACROS
		goto mouse_moved; /* bypass test below */
	}

	if(icb.state & MMOVE) /* this test is for macro input which may flag */
	{
mouse_moved:

#endif /* INPUT_MACROS */

		/* load values and apply offset */
		get_menucursorxy();

		if (do_cursor && (icb.cx != icb.lastcx || icb.cy != icb.lastcy))
			MOVECURSOR();
	}

	/* set inverted button "up" flags on the lo-word hipri buttons */
	SET_BUPBITS(icb.state);

	icb.xorstate = icb.state^icb.ostate;


	if(icb.state & KEYHIT)
	{
		if(icb.do_hot_key)
		{
			ret = (*icb.do_hot_key)(&icb);
			CHECK_POP_ICB(&recurs);
			if(ret)
				icb.input_eaten |= KEYHIT;
		}
	}

	if(icb.xorstate & HI_BSTATES)
	{
		icb.state |= HISTATE;
		icb.xorstate |= HISTATE;
	}

	if((icb.hitstate = icb.xorstate & icb.state) & icb.waithit)
	{
#ifdef INPUT_MACROS
		if(icb.macro_mode == MAKE_MACRO)
		{
			put_macro(icb.macro_clocked 
						|| (icb.hitstate & icb.recflags & icb.waithit)
						|| icb.input_eaten );
			CHECK_POP_ICB(&recurs);
		}
#endif /* INPUT_MACROS */
		goto done_true;
	}

#ifdef INPUT_MACROS
	if(icb.macro_mode == MAKE_MACRO)
	{
		put_macro((icb.macro_clocked && (icb.hitstate & MMOVE)) 
				   || icb.input_eaten);
		CHECK_POP_ICB(&recurs); 
	}
#endif /* INPUT_MACROS */

done_false:
	check_waitasks();
	ret = FALSE;
	goto done;
done_true:
	ret = TRUE;
done:

	if(icb.input_eaten) /* remove it from global data */
	{
		icb.state &= ~icb.input_eaten;
		icb.input_eaten = 0;

		/* Aaaack we have to resolve flags again if input was eaten above */

		icb.state &= ~HISTATE;
		if((icb.xorstate = icb.state^icb.ostate) & HI_BSTATES)
		{
			icb.state |= HISTATE;
			icb.xorstate |= HISTATE;
		}
		ret = (0 != ((icb.hitstate = icb.xorstate & icb.state) & icb.waithit));
	}
	icb.push = NULL;
	return(ret);
}

Boolean check_input(ULONG flags)
{
Boolean ret;

	/* will return TRUE if anything in flags is tested to have happend since
	 * last check or wait input */

	icb.waithit = flags;
	if(icb.mset.on && (icb.mcurs_up == 0))
	{
		DRAWCURSOR();
		++icb.mcurs_up;
		ret = _poll_input(0);
		UNDRAWCURSOR();
		--icb.mcurs_up;
	}
	else
		ret = _poll_input(1);

	icb.waithit = 0;
	return(ret);
}

void wait_input(ULONG waitflags)
{
Boolean do_cursor;

	if(WANTDRAWCURS)
	{
		DRAWCURSOR();
		do_cursor = ((waitflags & MMOVE) == 0);
	}
	else
		do_cursor = 1;

	icb.waithit = waitflags;
	for (;;)
	{
		if(_poll_input(do_cursor))
			break;
		mwaits();
	}
	icb.waithit = 0;

	HIDECURSOR();
}
void mac_wait_input(ULONG waitflags,ULONG recflags)

/* waitflags are what user wants to see, recflags are the things
 * (truncated to ONLY be a subset of waitflags cant specify what is NOT in
 *	 Waitflags )
 * we want to be sure macro records even if not in realtime mode.
 * ie: the bare essentials to make code function */
{
	icb.recflags = recflags;
	wait_input(waitflags);
	icb.recflags = ANY_INPUT;
}
Errcode mac_vsync_wait_input(ULONG waitflags, ULONG recflags, SHORT fields)
/* recflags will force these items to be recorded in the macro even if timed
 * out, flags are what will satisfy the wait without a timeout, 
 * if waitflags are statisfied macro will be recorded at least.
 * will wait at least one vsync */
{
Errcode err;
Boolean do_cursor;
SHORT omposx;
SHORT omposy;

	omposx = icb.sx;
	omposy = icb.sy;

	if(WANTDRAWCURS)
	{
		DRAWCURSOR();
		do_cursor = ((waitflags & MMOVE) == 0);
	}
	else
		do_cursor = 1;

	icb.waithit = waitflags;
	for(;;)
	{
		mwaits();
		if(--fields < 1)
		{
			/* set mouse position to old mouse mouse position, this
			 * will force the poller to set the MMOVE flag and record
			 * a macro if that is in recflags.
			 */
			icb.sx = omposx;
			icb.sy = omposy;

			icb.waithit |= recflags;
			_poll_input(do_cursor);

			if(JSTHIT(waitflags))
				err = 0;
			else
				err = Err_timeout;
			break;
		}
		else if(_poll_input(do_cursor))
		{
			err = 0;
			break;
		}
	}
	HIDECURSOR();
	icb.waithit = 0;
	return(err);
}
Errcode vsync_wait_input(ULONG flags, SHORT fields)
{
	return(mac_vsync_wait_input(flags,flags,fields));
}
static Errcode _wait_timeout(ULONG timeout_1000)

/* wait until a certain time.  Return Err_timeout if timeout was satisfied.
 * Return 0 if they hit what is set in waithit. always checks 
 * input will guarantee 1 to 1 number of input checks in macro file 1 check
 * for each invocation will reset waithit to 0 on exit negative timeouts
 * are ignored. Time units are millisecs or 1/1000 sec. */
{
Errcode ret;
ULONG lt, t;
Boolean do_cursor;

#ifdef INPUT_MACROS
	BYTE macro_mode;
	BYTE used_one_macro;
#endif /* INPUT_MACROS */

	if(WANTDRAWCURS)
	{
		DRAWCURSOR();
		do_cursor = ((icb.waithit & MMOVE) == 0);
	}
	else
		do_cursor = 1;

	lt = pj_clock_1000();

#ifdef INPUT_MACROS
	/* suspend macro recording while looping */
	if((macro_mode = icb.macro_mode) == MAKE_MACRO)
		icb.macro_mode &= ~MACRO_OK;

	used_one_macro = FALSE;
#endif /* INPUT_MACROS */

	for (;;)
	{
#ifdef INPUT_MACROS
		if(used_one_macro)
		{
			check_waitasks(); /* _poll_input() would do this so try to keep
							   * number of checks about equal */
		}
		else /* only check input first time if reading macro */
		{
			if(_poll_input(do_cursor))
			{
				ret = Success;
				break;
			}
			used_one_macro = (icb.macro_mode == USE_MACRO);
		}

#else /* not INPUT_MACROS */

		if(_poll_input(do_cursor))
		{
			ret = Success;
			break;
		}

#endif /* INPUT_MACROS */

		t = pj_clock_1000();
		if((LONG)timeout_1000 <= 0) 
		{
			ret = Err_timeout;
			break;
		}
		timeout_1000 -= (ULONG)(t - lt);
		lt = t;

		/* use vsync if time is > one field */
		if(timeout_1000 > icb.clocks_per_field)  
			mwaits();
		else if(timeout_1000 > 0x0f)
		{
			/* busy wait on 62nd of a sec not synced to beam */
			t &= ~(0x0f);
			while(t == ~(0xf)&pj_clock_1000());		
		}
	}

	HIDECURSOR();

#ifdef INPUT_MACROS
	if(macro_mode == MAKE_MACRO)
	{
		/* got input, record state, Timed out, add 1 to count */
		icb.macro_mode |= MACRO_OK;
		put_macro(ret == Success); 
	}
#endif /* INPUT_MACROS */

	icb.waithit = 0;
	return(ret);
}
Errcode timed_wait_input(ULONG waitflags, ULONG timeout_1000)

/* will wait for input until the timeout is reached, if timeout is reached
 * returns Err_timeout, otherwise input is hit and returns Success, If a macro
 * is being read input recieved will be the same even if the timeout period is
 * eliminated */
{
#ifdef INPUT_MACROS
	if(icb.macro_mode == USE_MACRO && !icb.macro_clocked)
		timeout_1000 = 0;
#endif /* INPUT_MACROS */

	icb.waithit = waitflags;
	return(_wait_timeout(timeout_1000));
}
Errcode wait_til(ULONG clock_1000)
{
	icb.waithit = MBRIGHT|KEYHIT;
	return(_wait_timeout(clock_1000 - pj_clock_1000()));
}

int anim_wait_input(ULONG waitflags, ULONG forceflags, 
					int maxfields, FUNC func, void *funcdata)

/* Calls func when entered, and each time the input in forceflags is detected
 * and at least every maxfields sync count.
 * unles maxfields <= 0 then it will only call on forceflags. If maxfields is 0
 * and forceflags is 0 it will wait forever. This should not be done.
 *
 * When func returns non 0 (int) the loop is terminated and the int returned.
 *
 * The input state is valid for each call to func and will remain so when 
 * func returns non zero, and anim_wait_input() exits. 
 *
 * If waitflags is satisfied the loop is broken.
 *
 * The mouse will be un-displayed when func is called.
 *
 * The func's intent is to produce
 * animated cursors and the like and should not do any input waits 
 * but can check the input state and read values in the icb which are valid */
{
int ret;
int fcount;
int cursor_was_up;
Boolean do_cursor;

	icb.waithit = waitflags;

	if(maxfields <= 0)
		maxfields = -1; /* it will wait a good long time this way */
	fcount = 0;

	if(WANTDRAWCURS)
	{
		DRAWCURSOR();
		cursor_was_up = 0;
		do_cursor = (((waitflags|forceflags) & MMOVE) == 0);
	}
	else
	{
		do_cursor = 1;
		cursor_was_up = 1;
	}


	for (;;)
	{
		mwaits(); /* leave cursor up and wait a field */

		if(fcount-- == 0) 
		{
		/* count used up do function but check input with no mouse */
			_poll_input(cursor_was_up);
			icb.hitstate |= ICB_TIMEOUT;
			fcount = maxfields;
			goto dofunc;
		}
		else
			_poll_input(do_cursor);

		if(forceflags & icb.hitstate) /* force execution of function */
		{
			if(!(forceflags & ICB_TIMEOUT))
				fcount = maxfields;
	dofunc:
			HIDECURSOR();
			if(0 != (ret = (*func)(funcdata)))
				break;
			SHOWCURSOR();
		}
		if(waitflags & icb.hitstate)
		{
			HIDECURSOR();
			break;
		}
	}
	icb.waithit = 0;
	return(ret);
}
