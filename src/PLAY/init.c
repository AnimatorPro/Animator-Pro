/* init.c  Routines to take the PC into and out of graphics mode, 
	find the mouse,  stow away the startup-directory, etc. */
#include <stdio.h>
#include <dos.h>
#include "jimk.h"
#include "init.str"


extern WORD mouse_connected;
void interrupt c_break();
void interrupt (*old_cbreak)();

char cbrk_hit;
char cbrk_int_in;

int ivmode;
char init_drawer[71];
char drawer[71];
int int_in; /* signals interrupts set */

/* The color map and a handle on the VGA screen */
extern UBYTE sys_cmap[COLORS*3];
Video_form vf = { 0,0,XMAX,YMAX,Raster_line(XMAX),VGA_SCREEN,sys_cmap, };






/* set us to 13H 320x200x256 */
new_video()
{
union regs r;

r.b.ah = 0;
r.b.al = 0x13;
sysint(0x10,&r,&r);
}




init_sys()
{
WORD i;
long t1, dt;
union regs r;

/* get old video mode */
r.b.ah = 0xf;
sysint(0x10, &r, &r);
ivmode = r.b.al;

new_video();

/* make sure made int into 13H */
r.b.ah = 0xf;
sysint(0x10, &r, &r);
if (r.b.al != 0x13)
	{
	puts(init_100 /* "Not a VGA/MCGA display, sorry" */);
	return(0);
	}
make_current_drawer();
strcpy(init_drawer, drawer);

if (!init_input()) 
	mouse_connected=0;
else mouse_connected=1;

setint(); /* these lines added -- Setting interrupt handler! */
fastint();
int_in=1;

init_de();

/* OPENDUMP();    */
get_devices();

see_cmap();
find_colors();

ctrl_brk(c_break); /* nullify control break */
return(1);
}


ctrl_brk(func)
void interrupt (*func)();
{
old_cbreak = getvect( 0x23 );
setvect ( 0x23 , func );
cbrk_int_in = 1;
}


void interrupt 
c_break()
{
union regs r;

/*cbrk_hit = 1; */
r.b.ah = 0;
sysint(0x16,&r,&r);
/* return(1); */
}


#ifdef EVER
cbrk_acknowledge()
{
cbrk_hit = 0;
}
#endif EVER




/* set old video mode */
old_video()
{
union regs r;
r.b.ah = 0;
r.b.al = ivmode;
sysint(0x10,&r,&r);
}






cleanup()
{
/* CLOSEDUMP();  */
change_dir(init_drawer);
old_video();
if (int_in)
	{
	Restoreint();
	}
if (cbrk_int_in)
	setvect(0x23, old_cbreak);
}

