/* convmove.c - routines to implement the move and slide functions */

#include "commonst.h"
#include "convert.h"
#include "fli.h"




static void move_cel(Rcel *c, int idx, int idy)
/*****************************************************************************
 * Have user move around position of cel with mouse 
 ****************************************************************************/
{
int ix,iy;
int ocx, ocy;
int dx,dy;

for (;;)
	{
	wait_click();		/* wait for first click */
	if (!JSTHIT(MBPEN))
		goto OUT;
	ix = icb.mx;
	iy = icb.my;
	ocx = c->x;
	ocy = c->y;
	dx = dy = 0;
	for (;;)
		{
		soft_status_line("!%-3d%-3d", "ctop_moved", dx+idx, dy+idy);
		wait_any_input();
		if(JSTHIT(KEYHIT|MBRIGHT))	/* CANCEL */
			{
			c->x = ocx;
			c->y = ocy;
			conv_see_cel(c);
			goto OUT;
			}
		if (JSTHIT(MBPEN))			/* SELECT */
			{
			break;
			}
		dx = icb.mx - ix;
		dy = icb.my - iy;
		c->x = ocx + dx;
		c->y = ocy + dy;
		conv_see_cel(c);
		}
	idx += icb.mx - ix;
	idx += icb.my - iy;
	}
OUT:
	cleanup_toptext();
}

void conv_move()
{
move_cel(cs.ifi.cel,0,0);
}

static void slide_with_mouse()
/*****************************************************************************
 * Have user set slide x/y parameters with mouse.
 ****************************************************************************/
{
Rcel *c = cs.ifi.cel;
int ox = c->x, oy = c->y;

c->x += cs.slidex;			/* fold in amount already slid */
c->y += cs.slidey;
conv_see_cel(c);			/* and display it in that position */
move_cel(c,cs.slidex,cs.slidey);	/* let user move it some */
cs.slidex += c->x - ox;		/* store total slide amounts */
cs.slidey += c->y - oy;
c->x = ox;					/* restore cel to it's pre-slid position */
c->y = oy;
conv_see_cel(c);			/* and redisplay it there */
}

static int sl_ox, sl_oy;

static Errcode slide_seek(int ix, void *data)
/*****************************************************************************
 * Draw cel at the slide position for a specific frame.
 ****************************************************************************/
{
int dframes = cs.slide_frames - cs.slide_complete;
Rcel *cel = cs.ifi.cel;
int fr2;
Errcode err;

if (data == NULL)	/* data pointer indicates if it's abortable */
	{
	if ((err = soft_abort("conv_abort")) < Success)
		return(err);
	}
if (dframes <= 0)
	dframes = 1;
fr2 = dframes/2;
cel->x = sl_ox + (ix*cs.slidex+fr2)/dframes;
cel->y = sl_oy + (ix*cs.slidey+fr2)/dframes;
conv_see_cel(cel);
return(Success);
}

static void preview_slide()
/*****************************************************************************
 * Display the cel moving across the screen with current slide settings
 * until right click or key is hit.
 ****************************************************************************/
{
int i;
Rcel *cel = cs.ifi.cel;
Boolean abortable = FALSE;

hide_mouse();
sl_ox = cel->x;
sl_oy = cel->y;
for (;;)
	{
	for (i=0; i<cs.slide_frames; i++)
		{
		slide_seek(i,&abortable);
		check_input(ANY_INPUT);
		if(JSTHIT(KEYHIT|MBRIGHT))
			goto OUT;
		wait_a_jiffy(1);
		}
	}
OUT:
cel->x = sl_ox;
cel->y = sl_oy;
conv_see_cel(cel);
show_mouse();
}


static void render_slide()
/*****************************************************************************
 * Ask user for filename to save sliding flic to, and then go slide
 * it.
 ****************************************************************************/
{
Rcel *cel = cs.ifi.cel;

sl_ox = cel->x;
sl_oy = cel->y;
save_a_flic(fli_pdr_name,NULL,cs.slide_frames,slide_seek);
cel->x = sl_ox;
cel->y = sl_oy;
conv_see_cel(cel);
}


void qconv_slide()
/*****************************************************************************
 * Put up slide menu.
 ****************************************************************************/
{
USHORT mdis[8];
short temp;

if (cs.slide_frames <= 0)
	cs.slide_frames = 50;
for (;;)
	{
	/* set up asterisks */
	clear_mem(mdis, sizeof(mdis));
	if (cs.slide_complete)
		mdis[4] |= QCF_ASTERISK;
	switch (soft_qchoice(mdis, "!%d%d%d", "conv_slide",
						 cs.slidex,cs.slidey,cs.slide_frames))
		{
		case 0:
			slide_with_mouse();
			break;
		case 1:
			temp = cs.ifi.cel->width;
			soft_qreq_number(&cs.slidex, -temp, temp, "conv_slidex");
			break;
		case 2:
			temp = cs.ifi.cel->height;
			soft_qreq_number(&cs.slidey, -temp, temp, "conv_slidey");
			break;
		case 3:
			soft_qreq_number(&cs.slide_frames, 2, 100, "conv_slidef");
			if (cs.slide_frames < 1)
				cs.slide_frames = 1;
			if (cs.slide_frames > MAXFRAMES)
				cs.slide_frames = MAXFRAMES;
			break;
		case 4:
			cs.slide_complete = !cs.slide_complete;
			break;
		case 5:
			preview_slide();
			break;
		case 6:
			render_slide();
			break;
		default:
			return;
		}
	}
}
