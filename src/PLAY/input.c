
/* input.c - where's the mouse?  What does the keyboard have to say?
   What time is it?  For the answers to these and other questions read on. */

#include "jimk.h"

/* These variable contain the global input state */
WORD uzx,uzy;	/* unzoomed xy */
WORD lastx, lasty;
WORD mouse_on = 1;
WORD mouse_moved;	/* mouse not same as last time - x,y or buttons */
WORD mouse_button, omouse_button;
WORD key_hit;
WORD key_in;
WORD firstx, firsty;
WORD mouse_connected;

#define ZEROFLAG	64
static struct regval {int ax,bx,cx,dx,si,di,ds,es;} mouser;

/*  Registers defined as mouse input and output values per Microsoft Mouse
    Installation and Operation Manual 8808-100-00 P/N 99F37B Chapter 4	*/
#define m1 mouser.ax
#define m2 mouser.bx
#define m3 mouser.cx
#define m4 mouser.dx
#define MOUSEINT	51
static WORD mscale = 4;
static WORD umouse_x, umouse_y;	/* unscaled mousexy */

show_mouse()
{
if (!mouse_connected) return;
mouse_on = 1;
}

hide_mouse()
{
if (!mouse_connected) return;
mouse_on = 0;
}


/* wait until start of vertical blank */
wait_sync()
{
wait_novblank();
wait_vblank();
}


wait_a_jiffy(j)
int j;
{
long l;

l = get80hz()+j;
for (;;)
	{
	if (get80hz() >= l)
		break;
	}
}


static
/*FCN*/mouse(fcn)	/* issue a mouse driver software interrupt of some sort */
  int fcn;
{
	m1 = fcn;
	sysint(MOUSEINT, &mouser, &mouser);
}

init_input()
{
char *mouse_int;
char	c;
char *bufs[6];
char buf[40];

m1 = m2 = 0;
mouse(0);		/* Initialize the mouse  */
if (m1 != -1)
	{
	return(0);
	}
return(1);
}



char umouse[256];	/* buffer to hold pixels behind the mouse cursor */
static WORD sx, sy;	/* xy position of saved buffer */


/* save under cursor */
scursor()
{
if (!mouse_connected) return;
sx = uzx-8;
sy = uzy-8;
blit8(16,16,sx,sy,vf.p,vf.bpr,
	0,0,umouse, 16);
}

/* restore area under cursor */
ucursor()
{
if (!mouse_connected) return;
blit8(16,16,0,0,umouse,16,
	sx,sy,vf.p,vf.bpr);
}

/* display the cursor */
ccursor()
{
if (!mouse_connected) return;
if (!mouse_on) return;
a1blit(16, 16, 0, 0, black_cursor, 2, sx, sy, vf.p,
	vf.bpr, sblack);
a1blit(16, 16, 0, 0, white_cursor, 2, sx, sy, vf.p,
	vf.bpr, sbright);
}

#define dcursor() {scursor();ccursor();}

static char reuse;

/* set flag to reuse current input */
reuse_input()
{
reuse = 1;
}


/* Record the mouse/keyboard input state in the variables uzx, uzy,
   key_hit, key_in, and mouse_button.  (uzx is for un-zoomed-x ... not
   really a good name.  Used to be mouse_x, mouse_y, but then things
   got complicated in vpaint with input macros, zoom, grid-lock etc.) */
c_input()
{
unsigned WORD w;
register WORD *a;
unsigned long l;
WORD mouse_color;
union regs r;


if (reuse)
	{
	reuse = 0;
	return;
	}
lastx = uzx;
lasty = uzy;
omouse_button = mouse_button;
key_hit = 0;
mouse(3);
mouse_button = m2 & ((1 << 2) - 1);	/* Extract the button bits */
mouse(11);
umouse_x += m3;
umouse_y += m4;
r.b.ah = 0x1;
if (!(sysint(0x16,&r,&r)&ZEROFLAG))
	{
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
if (mouse_on && !(uzx == lastx && uzy == lasty))
	{
	mouse_moved = 1;
	ucursor();
	dcursor();
	}
if (mouse_button != omouse_button)
	{
	mouse_moved = 1;
	}
}



/* Go grab next input (mouse & keyboard).  Don't wait for anything if
   it's not already there */
check_input()
{
dcursor();
c_input();
ucursor();
}


/* return after count clock ticks, or have new mouse position/key press */
vsync_input(count)
WORD count;
{
dcursor();
while (--count >= 0)
	{
	wait_sync();
	c_input();
	if (mouse_moved || key_hit)
		break;
	}
ucursor();
}

/* wait for new mouse or keyboard input */
wait_input()
{
dcursor();
for (;;)
	{
	c_input();
	if (mouse_moved || key_hit)
		break;
	}
ucursor();
}

/* wait until left mouse button is up */
wait_penup()
{
dcursor();
for (;;)
	{
	if (!PDN)
		break;
	c_input();
	}
ucursor();
}

/* wait until right mouse button is up */
wait_rup()
{
dcursor();
for (;;)
	{
	if (!RDN)
		break;
	c_input();
	}
ucursor();
}

/* wait until a mouse button or a keyboard key is pressed */
wait_click()
{
dcursor();
for (;;)
	{
	c_input();
	if (key_hit || RJSTDN || PJSTDN)
		break;
	}
ucursor();
}


check_button()
{
if (!mouse_connected) return;
omouse_button = mouse_button;
mouse(3);
mouse_button = m2 & ((1 << 2) - 1);	/* Extract the button bits */
}

