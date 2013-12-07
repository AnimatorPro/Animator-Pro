/* sys_dos.c */

#include "io_.h"

#define ZEROFLAG	64

union regs mouse_regs;

static WORD mscale = 4;
char got_mouse;

extern WORD reuse;
extern char usemacro;
extern char zoomcursor;
extern WORD lmouse_x, lmouse_y;

extern void get_macro(void);
extern void next_histrs(void);

/*FCN*/mouse_int(int fcn)
{
	mouse_regs.w.ax = fcn;
	sysint(MOUSEINT, &mouse_regs, &mouse_regs);
}

void
c_poll_input(void)
{
	unsigned WORD w;
	register WORD *a;
	unsigned long l;
	WORD mouse_color;
	union regs r;

	if (reuse) {
		reuse = 0;
		return;
	}

	lastx = uzx;
	lasty = uzy;
	omouse_button = mouse_button;
	key_hit = 0;

	if (got_mouse) {
		switch (vconfg.dev_type) {
			case 0: /* mousey */
				mouse_int(3);
				/* Extract the button bits */
				mouse_button = mouse_regs.w.bx & ((1 << 2) - 1); 
				mouse_int(11);
				umouse_x += mouse_regs.w.cx;
				umouse_y += mouse_regs.w.dx;
				break;
			case 1: /* where's my summa tablet? */
				summa_get_input();
				break;
#ifdef WACOM
			case 2:
				wacom_get_input();
				break;
#endif /* WACOM */
		}
		r.b.ah = 0x1;
		if (!(sysint(0x16,&r,&r)&ZEROFLAG))
		{
			key_hit = 1;
			r.b.ah = 0;
			sysint(0x16,&r,&r);
			key_in = r.w.ax;
		}
	}
	else {
		r.b.ah = 2;
		sysint(0x16,&r,&r);
		mouse_button = 0;
		if (r.b.al & 0x2) /* pendown on alt */
			mouse_button |= 0x1;
		if (r.b.al & 0x1) /* right button on control */
			mouse_button |= 0x2;
		r.b.ah = 0x1;
		if (!(sysint(0x16,&r,&r)&ZEROFLAG)) { /* snoop for arrow keys... */
			w = 1;
			switch (r.w.ax) {
				case LARROW:
					umouse_x += -4*mscale;
					break;
				case RARROW:
					umouse_x += 4*mscale;
					break;
				case UARROW:
					umouse_y += -4*mscale;
					break;
				case DARROW:
					umouse_y += 4*mscale;
					break;
				default:
					w = 0;
					break;
			}
			r.b.ah = 0;
			sysint(0x16,&r,&r);
			if (!w) { /* eat character if arrow... */
				key_hit = 1;
				key_in = r.w.ax;
			}
		}
	}

	/* clip unscaled mouse position and set scaled mouse_x and mouse_y */
	if (umouse_x < 0)
		umouse_x = 0;
	if (umouse_x > 319*mscale)
		umouse_x = 319*mscale;
	if (umouse_y < 0)
		umouse_y = 0;
	if (umouse_y > 199*mscale)
		umouse_y = 199*mscale;
	uzx = (umouse_x/mscale);
	uzy = (umouse_y/mscale);
	if (usemacro)
		get_macro();

	grid_x = uzx;
	grid_y = uzy;
	if (vs.zoom_mode) {
		grid_x = vs.zoomx + uzx/vs.zoomscale;
		grid_y = vs.zoomy + uzy/vs.zoomscale;
	}

	if (key_hit) {
		switch (key_in) {
#ifdef DEVEL
			case 0x3c00: /* F2 */
				save_gif("vsnapshot.gif", &vf);
				key_hit = 0;
				break;
#endif /* DEVEL */
			case 0x3d00: /* F3 */
				vs.mkx = grid_x;
				vs.mky = grid_y;
				mouse_button |= 0x1;
				key_hit = 0;
				break;
			case 0x3e00: /* F4 */
				grid_x = vs.mkx;
				grid_y = vs.mky;
				init_hr(); /* for GEL brush */
				mouse_button |= 0x1;
				key_hit = 0;
				break;
		}
	}
	get_gridxy();
	next_histrs();
	mouse_moved = 0;
	if (!(uzx == lastx && uzy == lasty)) {
		if (!zoomcursor || uzy/vs.zoomscale != lasty/vs.zoomscale ||
				uzx/vs.zoomscale != lastx/vs.zoomscale) {
			lmouse_x = lastx;
			lmouse_y = lasty;
			mouse_moved = 1;
			if (mouse_on) {
				ucursor();
				scursor();
				ccursor();
			}
		}
	}

	if (mouse_button != omouse_button) {
		lmouse_x = uzx;
		lmouse_y = uzy;
		mouse_moved = 1;
	}
	put_macro(clickonly);
}

void
c_wait_input(void)
{
	c_poll_input();
}

void
mwaits(void)
{
	union regs r;

	sysint(0x28, &r, &r); /* call idle interrupt */
	if (!usemacro) {
		wait_sync();
	}
}

/* either returns my interrupt clock, or system clock depending if
   interrupt in place */
long
get80hz()
{
	extern long _get80hz();
	extern long clock(void);

	if (vconfg.noint)
		return(clock()*4);
	else
		return(_get80hz());
}
