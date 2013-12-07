
/* input.c  -  This is the messy mouse section.  Responsible for updating
   the values in
   	key_in, key_hit, mouse_button, grid_x, grid_y, uzx, uzy,
   are updated every time c_input is called.  Macros are taken
   care of here by calling the appropriate routines in macro.c.
   The mouse coordinates are kept in both zoomed (for the paint tools:
   grid_x, grid_y) and unzoomed (for the menus: uzx, uzy) form.
   */


#include "jimk.h"
#include "blit8_.h"
#include "cblock_.h"
#include "clipit_.h"
#include "fli.h"
#include "flicmenu.h"

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
WORD reuse;
WORD lmouse_x, lmouse_y;
char zoomcursor;
PLANEPTR brushcursor;

extern char inwaittil;

static Point hr_buf[HSZ];
static WORD hr_n;

void scursor(void);
void ccursor(void);
static void
zoom_bitmap_blit(WORD w, WORD h,
		WORD sx, WORD sy, PLANEPTR spt, WORD sbpr,
		WORD dx, WORD dy, WORD color);
static void zo_line(int j, PLANEPTR spt, int xs, int xd, int yd, int color);
static void
unzoom_bitmap_blit(WORD w, WORD h,
		WORD sx, WORD sy, PLANEPTR spt, WORD sbpr, WORD dx, WORD dy);
static void unzo_line(int j, PLANEPTR spt, int xs, int xd, int yd);

extern void c_poll_input(void);
extern void c_wait_input(void);

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
void
next_histrs(void)
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

check_input()
{
dcursor();
flip_video();
c_poll_input();
ucursor();
}


static
vtinput(count, mm)
WORD count, mm;
{
recordall = 0;
dcursor();
flip_video();
while (--count >= 0)
	{
	mwaits();
	c_poll_input();
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
flip_video();
for (;;)
	{
	c_wait_input();
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
flip_video();
for (;;)
	{
	if (!PDN)
		break;
	mwaits();
	c_wait_input();
	}
ucursor();
recordall = 1;
}

#ifdef SLUFFED
wait_rup()
{
recordall = 0;
dcursor();
flip_video();
for (;;)
	{
	if (!RDN)
		break;
	mwaits();
	c_wait_input();
	}
ucursor();
recordall = 1;
}
#endif /* SLUFFED */

wait_click()
{
clickonly = 1;
if (mouse_on)
	dcursor();
flip_video();
for (;;)
	{
	c_wait_input();
	mwaits();
	if (key_hit || RJSTDN || PJSTDN)
		break;
	}
clickonly = 0;
if (mouse_on)
	ucursor();
}

static char umouse[256];
static WORD sx, sy;


void
scursor(void)
{
if (!zoomcursor)
	{
		int w = 16;
		int h = 16;
		int srcx = uzx-8;
		int srcy = uzy-8;
		int dstx = 0;
		int dsty = 0;

		if (clipblit2(&w, &h, &srcx, &srcy, vf.w, vf.h, &dstx, &dsty, 16, 16)) {
			blit8(w, h, srcx, srcy, vf.p, vf.bpr, dstx, dsty, umouse, 16);
			sx = uzx-8;
			sy = uzy-8;
		}
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

void
ccursor(void)
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

static void
zoom_bitmap_blit(WORD w, WORD h,
		WORD sx, WORD sy, PLANEPTR spt, WORD sbpr,
		WORD dx, WORD dy, WORD color)
{
spt += sy*sbpr;
while (--h >= 0)
	{
	zo_line(w, spt, sx, dx, dy, color);
	dy++;
	spt += sbpr;
	}
}

static void
zo_line(int j, PLANEPTR spt, int xs, int xd, int yd, int color)
{
while (--j >= 0)
	{
	if (spt[xs>>3] & bmasks[xs&7] )
		upd_zoom_dot(xd,yd,color);
	xs++;
	xd++;
	}
}

static void
unzoom_bitmap_blit(WORD w, WORD h,
		WORD sx, WORD sy, PLANEPTR spt, WORD sbpr, WORD dx, WORD dy)
{
spt += sy*sbpr;
while (--h >= 0)
	{
	unzo_line(w, spt, sx, dx, dy);
	dy++;
	spt += sbpr;
	}
}

static void
unzo_line(int j, PLANEPTR spt, int xs, int xd, int yd)
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
	c_poll_input();
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
	c_poll_input();	/* read last input state from macro file */
	if (RDN || key_hit)
		{
		ok = 0;
		}
	}
return(ok);
}

