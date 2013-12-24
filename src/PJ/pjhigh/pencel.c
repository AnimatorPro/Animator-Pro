#include "errcodes.h"
#include "pjbasics.h"

void clip_penwinrect(Rectangle *r)
/* dont let edge go past middle for big windows little ones dont let go over 
 * edge clip for non backdrop windows */
{
	if(r->width <= vb.scrcent.x)
		bclip_dim(&(r->x),&(r->width),0,vb.screen->wndo.width);
	else
	{
		if(r->x > vb.scrcent.x)
			r->x = vb.scrcent.x;
		else 
		{
			if((r->x += r->width) < vb.scrcent.x)
				r->x = vb.scrcent.x;
			r->x -= r->width;
		}
	}
	if(r->height <= vb.scrcent.y)
		bclip_dim(&(r->y),&(r->height),0,vb.screen->wndo.height);
	else
	{
		if(r->y > vb.scrcent.y)
			r->y = vb.scrcent.y;
		else
		{
			if((r->y += r->height) < vb.scrcent.y)
				r->y = vb.scrcent.y;
			r->y -= r->height;
		}
	}
}

Errcode set_pencel_size(SHORT width, SHORT height, SHORT xcen, SHORT ycen)
/* This sets the pencel (the drawing window) to a given size.
 * This should be called after a hide_mp(). */
{
Errcode err = Success;
WndoInit wi;

	if(vb.pencel)
	{
		if(vb.pencel->width == width		/* See if same size as before */
			&& vb.pencel->height == height)		
		{
			pj_set_rast(vb.pencel,0); /* clear it */
			return(Success);		  /* That was easy! */
		}
		fliborder_off();
		close_wndo((Wndo *)vb.pencel);
		vb.pencel = NULL;
	}

	pj_set_rast(&(vb.screen->wndo),0); /* clear screen wndo */

	if(height == vb.screen->wndo.height
		&& width == vb.screen->wndo.width)
	{
		vb.pencel = (Rcel *)(vb.screen); /* penwndo is whole screen */
	}
	else
	{
		vb.screen->wndo.doit = NULL;
		vb.screen->wndo.ioflags = 0;
		vb.screen->wndo.cursor = vb.screen->cursor;
		vb.screen->wndo.procmouse = NULL;

		clear_mem(&wi,sizeof(wi));
		wi.width = width;
		wi.height = height;
		wi.x = xcen + vb.scrcent.x - (width>>1);
		wi.y = ycen + vb.scrcent.y - (height>>1);
		wi.screen = vb.screen;
		wi.over = &(vb.screen->wndo);
		wi.flags = WNDO_NOCLEAR;

		if(height <= vb.screen->wndo.height
			&& width <= vb.screen->wndo.width)
		{
			wi.flags = (WNDO_BACKDROP | WNDO_NOCLEAR); /* cleared as screen */
			bclip0xy_rect((Rectangle *)&(wi.RECTSTART),
					      (Rectangle *)&(vb.screen->wndo.RECTSTART));
		}
		else
			clip_penwinrect((Rectangle *)&(wi.RECTSTART));

		err = open_wndo((Wndo **)&(vb.pencel),&wi);
	}
	return(err);
}

static void do_fliborder(Wndo *w,Boolean drawit)
/* Do the actual drawing of the rectangle surrounding the active
 * area of the screen (the pencel.)
 */
{
Pixel c1;

	if(drawit)
		c1 = vb.screen->SGREY;
	else
		c1 = 0;

	draw_quad(vb.screen,c1,w->x - 1,w->y - 1,w->width + 2,w->height + 2);
}
void draw_flibord()
{
	do_fliborder(PENWNDO,1);
}
void erase_flibord()
{
	do_fliborder(PENWNDO,0);
}
void fliborder_off()
/* Turn off the box surrounding the active drawing area.
 */
{
	if(PENWNDO->redraw)
	{
		erase_flibord(PENWNDO);
		PENWNDO->flags &= ~(WNDO_MUCOLORS);
	}
	PENWNDO->redraw = NULL;
}
void fliborder_on()
/* Set up things so that a rectangle is drawn around the active
 * drawing area when menus are up.
 */
{
	if( PENWNDO != &(vb.screen->wndo)
		&& (CODEOBSCURES !=  
			clipcode_crects((Cliprect *)&(PENWNDO->CRECTSTART),
							 (Cliprect *)&(vb.screen->wndo.CRECTSTART))))
	{
		PENWNDO->redraw = draw_flibord;
		PENWNDO->flags |= WNDO_MUCOLORS;
		redraw_wndo(PENWNDO);
	}
	else
		fliborder_off();
}

