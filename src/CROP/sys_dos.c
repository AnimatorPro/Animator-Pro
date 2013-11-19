/* sys_dos.c */

#include <dos.h>
#include "init.str"
#include "io_.h"
#include "io_dos.h"
#include "sys.h"

/*--------------------------------------------------------------*/
/* Init                                                         */
/*--------------------------------------------------------------*/

static int ivmode;  /* keep track of startup video mode */

int
init_system(void)
{
	ivmode = get_vmode();
	set_vmode(0x13);
	if (get_vmode() != 0x13) {
		puts(init_102);
		return 0;
	}

	if (!init_mem())
		return 0;

	if (!init_input())
		return 0;

	init_de();

	return 1;
}

void
old_video(void)
{
	set_vmode(ivmode);
}

void
cleanup(void)
{
	extern char init_drawer[];
	change_dir(init_drawer);
	old_video();
}

/*--------------------------------------------------------------*/
/* Input                                                        */
/*--------------------------------------------------------------*/

#define UP			0
#define ZEROFLAG	64

static WORD mscale = 4;
WORD umouse_x, umouse_y; /* unscaled mousexy */

extern char reuse;

static int
init_input(void)
{
	if (vconfg.dev_type == 1)
		return init_summa();
	else
		return init_mouse();
}

void
c_input(void)
{
	union regs r;

	if (reuse) {
		reuse = 0;
		return;
	}

	lastx = uzx;
	lasty = uzy;

	/* poll keyboard */
	key_hit = 0;
	r.b.ah = 0x1;
	if (!(sysint(0x16,&r,&r) & ZEROFLAG))
	{
		key_hit = 1;
		r.b.ah = 0;
		sysint(0x16,&r,&r);
		key_in = r.w.ax;
	}

	omouse_button = mouse_button;
	/* poll pointing device */
	if (vconfg.dev_type == 1)
		c_summa();
	else
		c_mouse();

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

	if (!(uzx == lastx && uzy == lasty)) {
		mouse_moved = 1;
		if (mouse_on) {
			ucursor();
			scursor();
			ccursor();
		}
	}

	if (mouse_button != omouse_button) {
		mouse_moved = 1;
	}
}

/*--------------------------------------------------------------*/
/* Timing                                                       */
/*--------------------------------------------------------------*/

long
get80hz()
{
	extern long clock();	/* turbo C clock call */

	return(clock()*4);
}
