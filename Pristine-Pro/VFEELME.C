
/* vfeelme.c - some input handling routines for Buttons. */

#include "errcodes.h"
#include "jimk.h"
#include "menus.h"


void toggle_pen(Button *m)
{
	vs.use_brush = !vs.use_brush;
	draw_buttontop(m);
}
