/* sys_dos.c */

#include "io_.h"
#include "io_dos.h"
#include "sys.h"

static int ivmode;  /* keep track of startup video mode */
static char int_in; /* keep track if interrupt is installed */

int
init_system(void)
{
	ivmode = get_vmode();
	set_vmode(0x13);
	if (get_vmode() != 0x13) {
		puts("Couldn't get a 320x200 256 color VGA screen");
		return 0;
	}

	setint();
	fastint();
	int_in = 1;
	return 1;
}

void
cleanup(void)
{
	set_vmode(ivmode);
	if (int_in) {
		Restoreint();
	}
}

unsigned int
strobe_keys(void)
{
#define ZEROFLAG	64

	union regs r;

	r.b.ah = 0x1;
	if (!(sysint(0x16, &r, &r) & ZEROFLAG)) {
		r.b.ah = 0;
		sysint(0x16, &r, &r);
		return r.w.ax;
	}
	else {
		return 0;
	}

#undef ZEROFLAG
}
