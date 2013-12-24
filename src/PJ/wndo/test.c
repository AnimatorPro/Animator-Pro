/* This is Jim's little test shell to help understand menu system. */

#include "errcodes.h"
#include "ptrmacro.h"
#include "memory.h"
#include "vdevice.h"
#include "vdevinfo.h"
#include "vdevcall.h"
#include "rastcall.h"
#include "wndo.h"

Errcode test_wscreen(Rcel *screen);

/**************** Stuff to Open MCGA Screen *************************/
int ivmode = -1;

void old_video()
/*
 * Restore video mode to start-up.
 * (aa_syslib wants this in the debugging version).
 */
{
if (ivmode != -1)
	pj_set_vmode(ivmode);
}

Errcode boxf(char *fmt,...)
/* this puts up a formated textbox for debugging etc bypassing input calls 
 * this only does printf() style formats */
/* (aa_stdiolib wants this) */
{
Errcode err;
va_list args;

	va_start(args, fmt);
	err = vprintf(fmt, args);
	va_end(args);
	return(err);
}

Errcode errline(Errcode err, char *format,...)
{
va_list args;

if (err < Success && err != Err_reported)
	{
	printf("Error %d", err);
	if (format != NULL)
		{
		va_start(args, format);
		err = vprintf(format, args);
		va_end(args);
		}
	printf("\n");
	err = Err_reported;
	}
return(err);
}

USHORT program_id = 1;
USHORT program_version = 0;

main()
{
	Errcode err;
	Vdevice *vdevice;
	Vmode_info mode_info;
	int width, height;
	int mode = 0;
	Rcel *screen;

	pj_set_gs();
	ivmode = pj_get_vmode(); /* save startup dos video mode */

	if ((err = pj_open_mcga_vdriver(&vdevice)) < Success)
		goto ERROR;

	if((err = pj_vd_verify_hardware(vdevice)) < Success)
		goto ERROR;

	if((err = pj_vd_get_mode(vdevice,mode,&mode_info)) < Success)
		goto ERROR;

	if ((err = alloc_display_rcel(vdevice, &screen
	, mode_info.width.actual, mode_info.height.actual, mode)) < Success)
		goto ERROR;
	err = test_wscreen(screen);

ERROR:
	old_video();
	printf("Err = %d\n", err);
	return err;
}
/**************** Stubs to get it to link. **********************/
void render_mask_blit(){}

ULONG randseed;

int random(void)
{
 	return((((ULONG)(randseed=
				((randseed * 0x41c64e6d)+0x3039)))>>10)&0x7fff);
}

/**************** Windows Test Code *************************/
static void hlines_on_rcel(Rcel *r)
{
int i;

for (i=0; i<r->height; ++i)
	pj_set_hline(r, i, 0, i, r->width);
}

/* Stuff for drawing a solid color circle. */
static int cicolor;

static Errcode ciline(int y, int left, int right,Raster *r)
{
	pj_set_hline(r, cicolor, left, y, right-left+1);
	return Success;
}

static void cidot(int x, int y, Raster *r)
{
pj_put_dot(r, cicolor, x, y);
}

static void rcel_target(Rcel *r)
{
int cenx = r->width/2, ceny = r->height/2;
int maxrad = Min(cenx, ceny);
int count = 10;
static int color_tab[2] = {32,0};
int i;
int rad;

for (i=count; i>0; --i)
	{
	rad = maxrad*i/count;
	cicolor = color_tab[i&1];
	dcircle(cenx, ceny, rad*2, cidot, r, ciline, r, TRUE);
	}
}


static void spray_dots(Rcel *w, int col_min, int col_max, int dots)
{
short xy[2];
int xcen = w->width/2;
int ycen = w->height/2;
int maxrad = Min(xcen,ycen);
int roff=0;
int sdcolor = col_min;

while (--dots >= 0)
	{
	polar( random()+roff++, random()%maxrad, xy);
	xy[0]+=xcen;
	xy[1]+= ycen;
	if (++sdcolor > col_max)
		sdcolor = col_min;
	pj_put_dot(w, sdcolor, xy[0], xy[1]);
	}
}


typedef struct moving_wndo
	{
	void (*init_me)(struct moving_wndo *this);
	WndoInit init;
	int dx,dy;
	void (*move_me)(struct moving_wndo *this);
	int count;
	Wndo *win;
	} Moving_wndo;
 

static void nada(Moving_wndo *this){}

static void mover_hlines(Moving_wndo *this)
{
hlines_on_rcel((Rcel *)(this->win));
}

static void mover_target(Moving_wndo *this)
{
rcel_target((Rcel *)(this->win));
}

static void mover_dots(Moving_wndo *this)
{
spray_dots((Rcel *)(this->win),32,36,16);
}


static void calc_bounce_rect(Moving_wndo *this, Rectangle *pos)
{
Wndo *win = this->win;

crect_torect((Cliprect *)(&win->CRECTSTART),pos);
pos->x += this->dx;
pos->y += this->dy;
if (pos->x < 0 
|| pos->x > win->W_screen->wndo.width - pos->width)
	{
	this->dx = -this->dx;
	pos->x += this->dx;
	}
if (pos->y < 0 
|| pos->y > win->W_screen->wndo.height - pos->height)
	{
	this->dy = -this->dy;
	pos->y += this->dy;
	}
}

static void bounce(Moving_wndo *this)
{
Rectangle pos;

calc_bounce_rect(this, &pos);
reposit_wndo(this->win,&pos,NULL);
}

static void bounce_dots(Moving_wndo *this)
{
spray_dots((Rcel *)(this->win),17,128,64);
bounce(this);
}

static void scroller(Moving_wndo *this)
{
Short_xy oset;
Rectangle pos;
Wndo *win = this->win;

get_wndo_oset(win,&oset);
oset.x += 1;
oset.y += 1;
calc_bounce_rect(this, &pos);
reposit_wndo(win,&pos,&oset);
spray_dots((Rcel *)win, 16, 32, 85);
}

static Moving_wndo movers[] = 
	{/*       init_me  {wid  hei    x    y maxw maxh}   dx dy      move_me*/
	{    mover_target, { 100, 100, 110,  50,   0,   0,}, 1, 2,      bounce},
	{            nada, {  50,  50, 135, 125,   0,   0,}, 2, 1, bounce_dots},
	{    mover_hlines, {  160, 50,  80,  75,   0,   0,}, 1,-1,      bounce},
	{    mover_target, {  40,  40, 155,  95, 350, 350,}, 1, 0,    scroller},
	{            nada, {  30,  30, 145,  85,   0,   0,}, 0, 0,  mover_dots},
	};

static void close_windows()
{
int i;
Moving_wndo *mov;

for (i=0; i<Array_els(movers); ++i)
	{
	mov = &movers[i];
	close_wndo(mov->win);
	mov->win = NULL;
	}
}

static Errcode open_windows(Wscreen *screen)
{
int i;
Errcode err;
Moving_wndo *mov;

for (i=0; i<Array_els(movers); ++i)
	{
	mov = &movers[i];
	mov->init.screen = screen;
	if ((err = open_wndo(&mov->win, &mov->init)) < Success)
		{
		close_windows();
		return err;
		}
	mov->init_me(mov);
	}
}

static Errcode update_windows()
{
int i;
Moving_wndo *mover;

for (;;)
	{
	if ((pj_key_in() & 0xff) == 0x1b)
		break;
	for (i=0; i<Array_els(movers); ++i)
		{
		mover = &movers[i];
		mover->move_me(mover);
		}
	}
return Success;
}

static Errcode move_windows(Wscreen *wscreen)
{
Errcode err;

if ((err = open_windows(wscreen)) >= Success)
	{
	err = update_windows();
	close_windows();
	}
return err;
}

static Errcode test_windows(Wscreen *wscreen)
{
return move_windows(wscreen);
}

Errcode test_wscreen(Rcel *screen)
/*
 * Open up Window Screen (attatching it to a simple Rcel), and
 * then call a deeper test routine.  
 */
{
WscrInit newscr;
Wscreen *wscreen;
Errcode err;

hlines_on_rcel(screen);
clear_mem(&newscr,sizeof(newscr));
newscr.max_wins = 10;
newscr.cel_a = screen;

if((err = open_wscreen(&wscreen,&newscr)) >= Success)
	{
	err = test_windows(wscreen);
	}
return err;
}
