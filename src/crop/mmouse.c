
/* microsoft mouse driver */
#include "jimk.h"
#include "mmouse.str"

extern WORD umouse_x, umouse_y;

static struct regval {int ax,bx,cx,dx,si,di,ds,es;} mouser;

/*  Registers defined as mouse input and output values per Microsoft Mouse
    Installation and Operation Manual 8808-100-00 P/N 99F37B Chapter 4	*/
#define m1 mouser.ax
#define m2 mouser.bx
#define m3 mouser.cx
#define m4 mouser.dx
#define MOUSEINT	51

static
/*FCN*/mouse(fcn)	/* issue a mouse driver software interrupt of some sort */
  int fcn;
{
	m1 = fcn;
	sysint(MOUSEINT, &mouser, &mouser);
}

init_mouse()
{
char **mouse_int;

m1 = m2 = 0;
mouse_int = NULL;
mouse_int += MOUSEINT;
if (mouse_int[0] != 0)
	mouse(0);		/* Initialize the mouse  */
if (m1 != -1)
	{
	early_err(mmouse_100 /* "Can't find the mouse, sorry, aborting\n" */);
	return(0);
	}
return(1);
}


c_mouse()
{
mouse(3);
mouse_button = m2 & ((1 << 2) - 1);	/* Extract the button bits */
mouse(11);
umouse_x += m3;
umouse_y += m4;
}

