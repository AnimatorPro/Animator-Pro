/* vfeelme.c - some input handling routines for Buttons. */

#include "jimk.h"
#include "errcodes.h"
#include "menus.h"
#include "pentools.h"

void toggle_pen(Button *m)
{
	vs.use_brush = !vs.use_brush;
	draw_buttontop(m);
}
