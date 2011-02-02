
/* input.c - where's the mouse?  What does the keyboard have to say?
   What time is it?  For the answers to these and other questions read on. */

#include "jimk.h"

/* These variable contain the global input state */
WORD uzx,uzy;	/* unzoomed xy */
WORD lastx, lasty;
WORD mouse_moved;	/* mouse not same as last time - x,y or buttons */
WORD mouse_button, omouse_button;
WORD key_hit;
WORD key_in;
WORD firstx, firsty;
WORD mouse_on = 1;

#define ZEROFLAG	64
static WORD mscale = 4;
WORD umouse_x, umouse_y;	/* unscaled mousexy */


/* wait until start of vertical blank */
wait_sync()
{
wait_novblank();
wait_vblank();
}


long
get80hz()
{
extern long clock();	/* turbo C clock call */

return(clock()*4);
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

init_input()
{
if (vconfg.dev_type == 1)
	return(init_summa());
else
	return(init_mouse());
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
union regs r;

if (reuse)
	{
	reuse = 0;
	return;
	}

lastx = uzx;
lasty = uzy;

/* poll keyboard */
key_hit = 0;
r.b.ah = 0x1;
if (!(sysint(0x16,&r,&r)&ZEROFLAG))
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
if (!(uzx == lastx && uzy == lasty))
	{
	mouse_moved = 1;
	if (mouse_on)
		{
		ucursor();
		dcursor();
		}
	}
if (mouse_button != omouse_button)
	{
	mouse_moved = 1;
	}
}


#ifdef SLUFFED
/* Go grab next input (mouse & keyboard).  Don't wait for anything if
   it's not already there */
check_input()
{
dcursor();
c_input();
ucursor();
}
#endif /* SLUFFED */ 


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

#ifdef SLUFFED
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
#endif /* SLUFFED */

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

hide_mouse()
{
mouse_on = 0;
}

show_mouse()
{
mouse_on = 1;
}


