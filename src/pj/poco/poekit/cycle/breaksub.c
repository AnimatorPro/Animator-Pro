/* cycle.c - a module called from poco to cycle the color map */
#include "errcodes.h"
#include "ptrmacro.h"
#include "pocorex.h"
#include "rcel.h"
#include "rastlib.h"
#include "gfx.h"


extern Rcel *GetPicScreen();

typedef struct rect
	{
	int color;
	int x,y;
	int width, height;
	} Rect;
typedef struct disk
	{
	int color;
	int x,y;
	int radius;
	} Disk;
void rect_draw(Popot pthis, int color)
{
Rect *this = pthis.pt;
SET_RECT(GetPicScreen(), color, this->x, this->y, 
	this->width, this->height);
}

oval_draw(Popot pthis, int color, int xrat, int yrat)
{
Disk *this = pthis.pt;
int xcen = this->x;
int ycen = this->y;
int rad = this->radius;
int err;
int derr, yerr, xerr;
int aderr, ayerr, axerr;
register int x,y;
int lasty;
Rcel *screen = GetPicScreen();

if (rad <= 0)
	{
	PUT_DOT(screen, color, xcen, ycen);
	return;
	}
xrat *= xrat;
yrat *= yrat;
err = 0;
x = rad;
lasty = y = 0;
for (;;)
	{
	if (y == 0)
		SET_HLINE(screen, color, xcen-x, ycen, x+x+1);
	else
		{
		if (lasty != y)
			{
			SET_HLINE(screen, color, xcen-x, ycen-y, x+x+1);
			SET_HLINE(screen, color, xcen-x, ycen+y, x+x+1);
			lasty = y;
			}
		}
	axerr = xerr = err + (-x-x+1)*yrat;
	ayerr = yerr = err + (y+y+1)*xrat;
	aderr = derr = yerr+xerr-err;
	if (aderr < 0)
		aderr = -aderr;
	if (ayerr < 0)
		ayerr = -ayerr;
	if (axerr < 0)
		axerr = -axerr;
	if (aderr <= ayerr && aderr <= axerr)
		{
		err = derr;
		x -= 1;
		y += 1;
		}
	else if (ayerr <= axerr)
		{
		err = yerr;
		y += 1;
		}
	else
		{
		err = xerr;
		x -= 1;
		}
	if (x < 0)
		break;
	}
}

disk_draw(Popot pthis, int color)
{
oval_draw(pthis, color, 1, 1);
}

Boolean rect_hit(Popot pthis, Popot pd)
{
Rect *this = pthis.pt;
Disk *d = pd.pt;
int rad = d->radius;
int x = this->x - d->x;
int y = this->y - d->y;

return		-rad <= x + this->width	&&
			rad >= x				&&
			-rad <= y + this->height &&
			rad >= y			;
}

typedef struct brick
	{
	Rect draw;
	char exists;
	} Brick;

int brick_collide(Popot pball, Popot pbrick, int count)
{
Disk *ball = pball.pt;
Brick *brick = pbrick.pt;
int bx = ball->x;
int by = ball->y;
int rad = ball->radius;
int x,y;
int i;

for (i=0; i<count; ++i)
	{
	if (brick->exists)
		{
		x = brick->draw.x - bx;
		y = brick->draw.y - by;
		if (-rad <= x + brick->draw.width  && rad >= x && 
		    -rad <= y + brick->draw.height && rad >= y)
			return(i);
		}
	++brick;
	}
return(-1);
}


static Lib_proto calls[] = {
	{ rect_draw,  "void rect_draw(struct rect *this, int color);"},
	{ disk_draw,  "void disk_draw(struct disk *this, int color);"},
	{ oval_draw,  
		"oval_draw(struct disk *this, int color, int xrat, int yrat);"},
	{ rect_hit,   "Boolean rect_hit(struct rect *this, struct disk *d);"},
	{ brick_collide, 
	 "int brick_collide(struct disk *ball, struct brick *brick, int count);"},
};

Hostlib _a_a_pocolib = { NULL, AA_POCOLIB, AA_POCOLIB_VERSION };

Pocorex rexlib_header = {
	{ 
	REX_POCO, 
	POCOREX_VERSION, 
	NOFUNC, NOFUNC, 
	&_a_a_pocolib, 
	POCO_REX_ID_STRING,
	},
	{
	NULL,
	"breaksub",
	calls,
	Array_els(calls),
	},
};


