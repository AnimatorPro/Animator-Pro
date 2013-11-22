
/* input.c  -  This is the messy mouse section.  Responsible for updating
   the values in
   	key_in, key_hit, mouse_button, grid_x, grid_y, uzx, uzy,
   are updated every time c_input is called.  Macros are taken
   care of here by calling the appropriate routines in macro.c.
   The mouse coordinates are kept in both zoomed (for the paint tools:
   grid_x, grid_y) and unzoomed (for the menus: uzx, uzy) form.
   */


#include "jimk.h"
#include "flicmenu.h"
#include "fli.h"


Vip aa_vip;
WORD gel_input;
WORD firstx, firsty;
WORD omouse_button;
WORD grid_x, grid_y;
WORD uzx,uzy;	/* unzoomed xy */
WORD lastx, lasty;
WORD mouse_moved;	/* mouse not same as last time - x,y or buttons */
WORD key_hit;
WORD key_in;
WORD mouse_on = 1;
static WORD reuse;
static WORD lmouse_x, lmouse_y;
char zoomcursor;
PLANEPTR brushcursor;

extern char inwaittil;

union regs mouse_regs;
/*  Registers defined as mouse input and output values per Microsoft Mouse
    Installation and Operation Manual 8808-100-00 P/N 99F37B Chapter 4	*/
#define MOUSEINT	51
static WORD mscale = 4;

char got_mouse;

static Point hr_buf[HSZ];
static WORD hr_n;

/* init input coordinate histerisus (sp).  Ie stuff for a basic lo-pass
   filter on the mouse/tablet coordinates */
extern Point hr_buf[HSZ];	/* in input.c */
extern WORD hr_n;	/* in input.c */

init_hr()
{
int i;
register Point *p;

p = hr_buf;
i = HSZ;
while (--i >= 0)
	{
	p->x = grid_x;
	p->y = grid_y;
	p++;
	}
hr_n = 0;
}


/* update historisis buffer with latest mouse coordinates, and return
   x/y position after hirstorisis smoothing in grid_x, grid_y */
static
next_histrs()
{
extern WORD gel_input;
int divs;
int i;
int n;
register Point *p;
long x,y;

p = hr_buf+hr_n;
p->x = grid_x;
p->y = grid_y;
n = hr_n;
hr_n -= 1;
if (hr_n < 0)
	hr_n = HSZ-1;
if (gel_input)
	{
	x = y = 0;
	divs = 0;
	i = HSZ;
	while (--i>=0)
		{
		if (n>=HSZ)
			n = 0;
		p = hr_buf+n;
		n++;
		x<<=1;
		y<<=1;
		divs<<=1;
		x+= p->x;
		y+= p->y;
		divs += 1;
		}
	grid_x = (x+divs/2)/divs;
	grid_y = (y+divs/2)/divs;
	}
}



reuse_input()
{
reuse = 1;
}

hide_mouse()
{
mouse_on = 0;
}

show_mouse()
{
mouse_on = 1;
}


#define dcursor() {scursor();ccursor();}
#define ZEROFLAG	64

get_gridxy()
{
if (vs.use_grid)
	{
	grid_x = ((grid_x - vs.gridx + vs.gridw/2)/vs.gridw*vs.gridw+vs.gridx);
	grid_y = ((grid_y - vs.gridy + vs.gridh/2)/vs.gridh*vs.gridh+vs.gridy);
	if (grid_x < 0)
		grid_x = 0;
	if (grid_x >= XMAX)
		grid_x = XMAX-1;
	if (grid_y < 0)
		grid_y = 0;
	if (grid_y >= YMAX)
		grid_y = YMAX-1;
	}
}

c_input()
{
unsigned WORD w;
register WORD *a;
unsigned long l;
WORD mouse_color;
union regs r;


AGAIN:
if (reuse)
	{
	reuse = 0;
	return;
	}
lastx = uzx;
lasty = uzy;
omouse_button = mouse_button;
key_hit = 0;
if (got_mouse)
	{
	switch (vconfg.dev_type)
		{
		case 0:	/* mousey */
			mouse_int(3);
			/* Extract the button bits */
			mouse_button = mouse_regs.w.bx & ((1 << 2) - 1); 
			mouse_int(11);
			umouse_x += mouse_regs.w.cx;
			umouse_y += mouse_regs.w.dx;
			break;
		case 1:	/* where's my summa tablet? */
			summa_get_input();
			break;
#ifdef WACOM
		case 2:
			wacom_get_input();
			break;
#endif WACOM
		}
	r.b.ah = 0x1;
	if (!(sysint(0x16,&r,&r)&ZEROFLAG))
		{
		key_hit = 1;
		r.b.ah = 0;
		sysint(0x16,&r,&r);
		key_in = r.w.ax;
		}
	}
else
	{
	r.b.ah = 2;
	sysint(0x16,&r,&r);
	mouse_button = 0;
	if (r.b.al & 0x2)	/* pendown on alt */
		mouse_button |= 0x1;
	if (r.b.al & 0x1)	/* right button on control */
		mouse_button |= 0x2;
	r.b.ah = 0x1;
	if (!(sysint(0x16,&r,&r)&ZEROFLAG))	/* snoop for arrow keys... */
		{
		w = 1;
		switch (r.w.ax)
			{
			case LARROW:
				umouse_x += -4*mscale;
				break;
			case RARROW:
				umouse_x += 4*mscale;
				break;
			case UARROW:
				umouse_y += -4*mscale;
				break;
			case DARROW:
				umouse_y += 4*mscale;
				break;
			default:
				w = 0;
				break;
			}
		r.b.ah = 0;
		sysint(0x16,&r,&r);
		if (!w)	/* eat character if arrow... */
			{
			key_hit = 1;
			key_in = r.w.ax;
			}
		}
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
if (usemacro)
	get_macro();
grid_x = uzx;
grid_y = uzy;
if (vs.zoom_mode)
	{
	grid_x = vs.zoomx + uzx/vs.zoomscale;
	grid_y = vs.zoomy + uzy/vs.zoomscale;
	}
if (key_hit)
	{
	switch (key_in)
		{
#ifdef DEVEL
		case 0x3c00:	/* F2 */
			save_gif("vsnapshot.gif", &vf);
			key_hit = 0;
			break;
#endif DEVEL
		case 0x3d00:	/* F3 */
			vs.mkx = grid_x;
			vs.mky = grid_y;
			mouse_button |= 0x1;
			key_hit = 0;
			break;
		case 0x3e00:	/* F4 */
			grid_x = vs.mkx;
			grid_y = vs.mky;
			init_hr();	/* for GEL brush */
			mouse_button |= 0x1;
			key_hit = 0;
			break;
		}
	}
get_gridxy();
next_histrs();
mouse_moved = 0;
if (!(uzx == lastx && uzy == lasty))
	{
	if (!zoomcursor || uzy/vs.zoomscale != lasty/vs.zoomscale ||
		uzx/vs.zoomscale != lastx/vs.zoomscale  )
		{
		lmouse_x = lastx;
		lmouse_y = lasty;
		mouse_moved = 1;
		if (mouse_on)
			{
			ucursor();
			dcursor();
			}
		}
	}
if (mouse_button != omouse_button)
	{
	lmouse_x = uzx;
	lmouse_y = uzy;
	mouse_moved = 1;
	}
put_macro(clickonly);
}

check_input()
{
dcursor();
c_input();
ucursor();
}


static
vtinput(count, mm)
WORD count, mm;
{
recordall = 0;
dcursor();
while (--count >= 0)
	{
	mwaits();
	c_input();
	if ((mm && mouse_moved) || key_hit)
		break;
	}
ucursor();
recordall = 1;
}

vsync_input(count)
WORD count;
{
vtinput(count, 1);
}

timed_input(count)
WORD count;
{
vtinput(count, 0);
}

wait_input()
{
recordall = 0;
dcursor();
for (;;)
	{
	c_input();
	if (mouse_moved || key_hit)
		break;
	mwaits();
	}
ucursor();
recordall = 1;
}

wait_penup()
{
recordall = 0;
dcursor();
for (;;)
	{
	if (!PDN)
		break;
	mwaits();
	c_input();
	}
ucursor();
recordall = 1;
}

#ifdef SLUFFED
wait_rup()
{
recordall = 0;
dcursor();
for (;;)
	{
	if (!RDN)
		break;
	mwaits();
	c_input();
	}
ucursor();
recordall = 1;
}
#endif SLUFFED

wait_click()
{
clickonly = 1;
if (mouse_on)
	dcursor();
for (;;)
	{
	c_input();
	mwaits();
	if (key_hit || RJSTDN || PJSTDN)
		break;
	}
clickonly = 0;
if (mouse_on)
	ucursor();
}


/*FCN*/mouse_int(fcn)
  int fcn;
{
	mouse_regs.w.ax = fcn;
	sysint(MOUSEINT, &mouse_regs, &mouse_regs);
}


static char umouse[256];
static WORD sx, sy;


static scursor()
{
if (!zoomcursor)
	{
	sx = uzx-8;
	sy = uzy-8;
	blit8(16,16,sx,sy,vf.p,vf.bpr,
		0,0,umouse, 16);
	}
else
	{
	sx = uzx/vs.zoomscale + vs.zoomx - 8;
	sy = uzy/vs.zoomscale + vs.zoomy - 8;
	}
}

ucursor()
{
if (zoomcursor)
	{
	unzoom_bitmap_blit(16, 16, 0, 0, white_cursor, 2, sx, sy);
	unzoom_bitmap_blit(16, 16, 0, 0, black_cursor, 2, sx, sy);
	if (brushcursor)
		unzoom_bitmap_blit(16, 16, 0, 0, brushcursor, 2, sx, sy);
	}
else
	blit8(16,16,0,0,umouse,16,
		sx,sy,vf.p,vf.bpr);
}

static
ccursor()
{
if (zoomcursor)
	{
	if (brushcursor)
		zoom_bitmap_blit(16, 16, 0, 0, brushcursor, 2, sx, sy, vs.ccolor);
	zoom_bitmap_blit(16, 16, 0, 0, black_cursor, 2, sx, sy, sblack);
	zoom_bitmap_blit(16, 16, 0, 0, white_cursor, 2, sx, sy, sbright);
	}
else
	{
	if (brushcursor)
		a1blit(16, 16, 0, 0, brushcursor, 2, sx, sy, vf.p,
			vf.bpr, vs.ccolor);
	a1blit(16, 16, 0, 0, black_cursor, 2, sx, sy, vf.p,
		vf.bpr, sblack);
	a1blit(16, 16, 0, 0, white_cursor, 2, sx, sy, vf.p,
		vf.bpr, sbright);
	}
}

static
zoom_bitmap_blit(w,h,sx,sy,spt,sbpr,dx,dy,color)
WORD w, h, sx, sy, sbpr,dx,dy,color;
register PLANEPTR spt;
{
spt += sy*sbpr;
while (--h >= 0)
	{
	zo_line(w, spt, sx, dx, dy, color);
	dy++;
	spt += sbpr;
	}
}

static
zo_line(j, spt, xs, xd, yd, color)
int j;
register int xs;
register PLANEPTR spt;
int xd,yd,color;
{
while (--j >= 0)
	{
	if (spt[xs>>3] & bmasks[xs&7] )
		upd_zoom_dot(xd,yd,color);
	xs++;
	xd++;
	}
}

static
unzoom_bitmap_blit(w,h,sx,sy,spt,sbpr,dx,dy)
WORD w, h, sx, sy, sbpr,dx,dy;
register PLANEPTR spt;
{
spt += sy*sbpr;
while (--h >= 0)
	{
	unzo_line(w, spt, sx, dx, dy);
	dy++;
	spt += sbpr;
	}
}

static
unzo_line(j, spt, xs, xd, yd)
int j;
register int xs;
register PLANEPTR spt;
int xd,yd;
{
PLANEPTR p;

p = zoom_form->p;
while (--j >= 0)
	{
	if (spt[xs>>3] & bmasks[xs&7] )
		upd_zoom_dot(xd,yd,getd(p,xd,yd));
	xs++;
	xd++;
	}
}


/* either returns my interrupt clock, or system clock depending if
   interrupt in place */
long
get80hz()
{
extern long _get80hz();
extern long clock(void);

if (vconfg.noint)
	return(clock()*4);
else
	return(_get80hz());
}

wait_sync()
{
wait_novblank();
wait_vblank();
}


static
mwaits()
{
union regs r;

sysint(0x28, &r, &r);	/* call idle interrupt */
if (!usemacro)
	{
	wait_sync();
	}
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
	mwaits();
	}
}


/* wait until a certain time.  Return 1 if timed out.  Return 0 if they
  hit a key or right button while waiting.  Fairly severly complicated
  by maintaining macros while this is happening (since may call c_input
  a variable number of times... */
wait_til(time)
long time;
{
int ok;
int odef;

ok = 1;
inwaittil = 1;	/* effectively squelch macro activity  */
for (;;)
	{
	c_input();
	if (RDN || key_hit)
		{
		ok = 0;
		break;
		}
	if (time <= get80hz())
		{
		break;
		}
	wait_sync();
	}
inwaittil = 0;	/* make macros happen again */
put_macro(0);	/* write out last input state into macro file */
if (usemacro)
	{
	c_input();	/* read last input state from macro file */
	if (RDN || key_hit)
		{
		ok = 0;
		}
	}
return(ok);
}

