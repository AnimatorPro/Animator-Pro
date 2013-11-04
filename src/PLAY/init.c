/* init.c  Routines to take the PC into and out of graphics mode, 
	find the mouse,  stow away the startup-directory, etc. */
#include <stdio.h>
#include <string.h>
#include "jimk.h"
#include "init.str"
#include "io_.h"
#include "sys.h"

extern WORD mouse_connected;

int ivmode;
char init_drawer[71];
char drawer[71];
int int_in; /* signals interrupts set */

/* The color map and a handle on the VGA screen */
extern UBYTE sys_cmap[COLORS*3];
Video_form vf = { 0,0,XMAX,YMAX,Raster_line(XMAX),VGA_SCREEN,sys_cmap,0 };


init_sys()
{
if (!init_system())
	return(0);

make_current_drawer();
strcpy(init_drawer, drawer);

/* OPENDUMP();    */
get_devices();

see_cmap();
find_colors();

return(1);
}

cleanup()
{
/* CLOSEDUMP();  */
change_dir(init_drawer);
shutdown_system();
}
