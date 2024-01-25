
/* input.c - where's the mouse?  What does the keyboard have to say?
   What time is it?  For the answers to these and other questions read on. */

#include "jimk.h"
#include "blit8_.h"
#include "io_.h"

/* These variable contain the global input state */
WORD uzx,uzy;	/* unzoomed xy */
WORD lastx, lasty;
WORD mouse_moved;	/* mouse not same as last time - x,y or buttons */
WORD mouse_button, omouse_button;
WORD key_hit;
WORD key_in;
WORD firstx, firsty;
WORD mouse_on = 1;

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

char umouse[256];	/* buffer to hold pixels behind the mouse cursor */
static WORD sx, sy;	/* xy position of saved buffer */


/* save under cursor */
scursor()
{
int dx,dy,w,h;
int lsx,lsy;

/* normal blit doesn't do source clipping, sigh */
w = 16;
h = 16;
lsx = sx = uzx-8;
lsy = sy = uzy-8;
dx = dy = 0;
if (sx < 0)
	{
	w += sx;
	dx = -sx;
	lsx = 0;
	}
if (sy < 0)
	{
	h += sy;
	dy = -sy;
	lsy = 0;
	}
blit8(w,h,lsx,lsy,vf.p,vf.bpr,
	dx,dy,umouse, 16);
}

/* restore area under cursor */
ucursor()
{
blit8(16,16,0,0,umouse,16,
	sx,sy,vf.p,vf.bpr);
}

/* display the cursor */
ccursor()
{
a1blit(16, 16, 0, 0, black_cursor, 2, sx, sy, vf.p,
	vf.bpr, sblack);
a1blit(16, 16, 0, 0, white_cursor, 2, sx, sy, vf.p,
	vf.bpr, sbright);
}

#define dcursor() {scursor();ccursor();}

char reuse;

/* set flag to reuse current input */
reuse_input()
{
reuse = 1;
}

#ifdef SLUFFED
/* Go grab next input (mouse & keyboard).  Don't wait for anything if
   it's not already there */
check_input()
{
dcursor();
flip_video();
c_input();
ucursor();
}
#endif /* SLUFFED */ 


/* return after count clock ticks, or have new mouse position/key press */
vsync_input(count)
WORD count;
{
dcursor();
flip_video();
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
flip_video();
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
flip_video();
for (;;)
	{
	if (!PDN)
		break;
	c_input();
	}
ucursor();
}

#ifdef SLUFFED
/* wait until right mouse button is up */
wait_rup()
{
dcursor();
flip_video();
for (;;)
	{
	if (!RDN)
		break;
	c_input();
	}
ucursor();
}
#endif /* SLUFFED */

/* wait until a mouse button or a keyboard key is pressed */
wait_click()
{
dcursor();
flip_video();
for (;;)
	{
	c_input();
	if (key_hit || RJSTDN || PJSTDN)
		break;
	}
ucursor();
}

hide_mouse()
{
mouse_on = 0;
}

show_mouse()
{
mouse_on = 1;
}


