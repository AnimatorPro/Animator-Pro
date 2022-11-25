/* sys_dos.c */

#include <dos.h>
#include "init.str"
#include "io_.h"
#include "io_dos.h"
#include "sys.h"

static int init_input(void);

/*--------------------------------------------------------------*/
/* Init                                                         */
/*--------------------------------------------------------------*/

static int ivmode;  /* keep track of startup video mode */
static char int_in; /* keep track if interrupt is installed */
static char cbrk_int_in;
static char cbrk_hit;
static void interrupt (*old_cbreak)();

extern WORD mouse_connected;

static void
ctrl_brk(void interrupt (*func)())
{
	old_cbreak = getvect(0x23);
	setvect(0x23, func);
	cbrk_int_in = 1;
}

static void interrupt
c_break()
{
	union regs r;

	/* cbrk_hit = 1; */
	r.b.ah = 0;
	sysint(0x16,&r,&r);
	/* return(1); */
}

#ifdef EVER
cbrk_acknowledge()
{
cbrk_hit = 0;
}
#endif /* EVER */

int
init_system(void)
{
	ivmode = get_vmode();
	set_vmode(0x13);
	if (get_vmode() != 0x13) {
		puts(init_100);
		return 0;
	}

	if (!init_input())  {
		mouse_connected = 0;
	}
	else {
		mouse_connected = 1;
	}

	init_de();

	setint();
	fastint();
	int_in = 1;

	ctrl_brk(c_break); /* nullify control break */

	return 1;
}

void
shutdown_system(void)
{
	set_vmode(ivmode);
	if (int_in) {
		Restoreint();
	}

	if (cbrk_int_in) {
		setvect(0x23, old_cbreak);
	}
}

/*--------------------------------------------------------------*/
/* Input                                                        */
/*--------------------------------------------------------------*/

/*  Registers defined as mouse input and output values per Microsoft Mouse
	Installation and Operation Manual 8808-100-00 P/N 99F37B Chapter 4 */
#define m1 mouser.ax
#define m2 mouser.bx
#define m3 mouser.cx
#define m4 mouser.dx
#define MOUSEINT	51

#define UP			0
#define ZEROFLAG	64

static struct regval {
	int ax, bx, cx, dx, si, di, ds, es;
} mouser;

static WORD mscale = 4;
static WORD umouse_x, umouse_y; /* unscaled mousexy */

extern char reuse;
extern char ctrl_hit;
extern char notice_keys;
extern int ascii_value;

static
/*FCN*/mouse(int fcn)
{
	m1 = fcn;
	sysint(MOUSEINT, &mouser, &mouser);
}

static int
init_input(void)
{
	char *mouse_int;
	char c;
	char *bufs[6];
	char buf[40];

	m1 = m2 = 0;
	mouse(0);		/* Initialize the mouse  */
	if (m1 != -1) {
		return 0;
	}
	return 1;
}

#ifndef EVER
void
get_key(void)
{
union regs r;

key_hit = 0;
r.b.ah = 0x1;
if (!(sysint(0x16,&r,&r)&ZEROFLAG))
	{
	key_hit = 1;
	r.b.ah = 0;
	sysint(0x16,&r,&r);
	key_in = r.w.ax;
	}
/** ldg */
/*  function 16H ah=2; returns in al the ROM BIOS keyobard flags */
ctrl_hit=UP;   /* ldg  ????????????????? */
r.b.ah = 0x02;
sysint(0x16, &r, &r);
ctrl_hit = (r.b.al & 0x04) ? 1: 0;
}
#endif /* EVER */

int
break_key(void)
{
	check_button();
	if (notice_keys && RDN)
		return 0;

	if (bioskey(1)) { /* a keystroke is waiting */
		key_hit = 1;
		key_in = bioskey(0); /* get the key */
		ctrl_hit = (bioskey(2) & CTRL); /* if was control key */
		ascii_value = ((key_in<<8)>>8);
		return(key_effect(key_in));
	}
	else {
		key_hit = 0;
		return 1;
	}
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
	mouse(3);
	mouse_button = m2 & ((1 << 2) - 1); /* Extract the button bits */
	mouse(11);
	umouse_x += m3;
	umouse_y += m4;
	r.b.ah = 0x1;
	if (!(sysint(0x16,&r,&r) & ZEROFLAG)) {
		key_hit = 1;
		r.b.ah = 0;
		sysint(0x16,&r,&r);
		key_in = r.w.ax;
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
	mouse_moved = 0;

	if (mouse_on && !(uzx == lastx && uzy == lasty)) {
		mouse_moved = 1;
		ucursor();
		scursor();
		ccursor();
	}

	if (mouse_button != omouse_button) {
		mouse_moved = 1;
	}
}

void
c_wait_input(void)
{
	c_poll_input();
}

void
check_button(void)
{
	if (!mouse_connected)
		return;

	omouse_button = mouse_button;
	mouse(3);
	mouse_button = m2 & ((1 << 2) - 1); /* Extract the button bits */
}
