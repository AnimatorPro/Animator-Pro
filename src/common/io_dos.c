/* io_dos.c */

#include "io_.h"
#include "io_dos.h"

void
set_vmode(int mode)
{
	union regs r;

	r.b.ah = 0;
	r.b.al = mode;
	sysint(0x10, &r, &r);
}

int
get_vmode(void)
{
	union regs r;

	r.b.ah = 0xf;
	sysint(0x10, &r, &r);
	return r.b.al;
}

void
flip_video(void)
{
}
