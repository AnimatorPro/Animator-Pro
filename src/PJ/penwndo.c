#include "jimk.h"
#include "errcodes.h"
#include "rastcurs.h"

extern int do_pen_tool(Wndo *w);

static Errcode alloc_undof(SHORT width,SHORT height)
{
	return(valloc_anycel(&undof,width,height));
}
void free_undof()
{
	pj_rcel_free(undof);
	undof = NULL;
}

static void penwin_procmouse(void)
/* procmouse function for the fli drawing window it handles grid for now */
{
	if(!vs.use_grid)
		return;

	grid_flixy(&icb.mx,&icb.my);
	if(mouseon_wndo(PENWNDO))
	{
		icb.cx = icb.mx + icb.mset.oset.x; /* have cursor track grid */ 
		icb.cy = icb.my + icb.mset.oset.y; 
		if(icb.cx == icb.lastcx && icb.cy == icb.lastcy)/* cancel mouse move */
			icb.state &= ~(MMOVE);
	}
}


void movefli_tool(void)
{
	hide_mp();
	load_wndo_mouset(NULL);
	set_cursor(&hand_cursor.hdr);
	draw_flibord();

	for(;;)
	{
		wait_input(MBPEN|MBRIGHT|KEYHIT);

		if(JSTHIT(KEYHIT))
			break;

		if(vs.zoom_open && curson_wndo(vl.zoomwndo))
		{
			if(zoom_handtool() < 0)
				break;
			continue;
		}

		if(JSTHIT(KEYHIT|MBRIGHT))
			break;

		if(curson_wndo(PENWNDO) || curson_wndo(&vb.screen->wndo))
		{
			if(move_penwndo() < 0)
				break;
		}
	}
	if(PENWNDO->redraw == NULL)
		erase_flibord();


	show_mp();
}

Errcode move_penwndo()
{
Wndo *fw = PENWNDO;
Errcode err;

	erase_flibord();
	if(fw->flags & WNDO_BACKDROP)
	{
		if(marqmove_wndo(fw, (Rectangle *)&(vb.screen->wndo.RECTSTART)))
			err = 0;
		else
		{
			if(JSTHIT(KEYHIT|MBRIGHT))
				err = Err_abort;
			else
				err = 1; /* didn't move */
		}
	}
	else 
		err = move_rear_wndo(fw,clip_penwinrect);

	if(err >= Success)
	{
		vl.flicent.x = vb.pencel->x + (vb.pencel->width>>1) - vb.scrcent.x;
		vl.flicent.y = vb.pencel->y + (vb.pencel->height>>1) - vb.scrcent.y;
		vs.flicentx = scale_vscoor(vl.flicent.x, vb.pencel->width);
		vs.flicenty = scale_vscoor(vl.flicent.y, vb.pencel->height);
	}
	draw_flibord();
	return(err);
}

Errcode set_penwndo_size(SHORT width, SHORT height)
/* This will set the penwndo to a size or reset it to a new size.
 * If it fails it will leave things at the old size and back out.
 * One should do a push_pics before this is called and a hide_mp()
 * and close every thing down.
 * The penwndo includes the pencel (the drawing screen) and the
 * undof (the backup screen.) */
{
Errcode err;
Rectangle osize;
void *cbuf;
static Boolean restore_size; /* for restore if error */


	if(undof 
		&& (undof->width != width 
			|| undof->height != height))
	{
		free_undof(); /* free old wrong size undo screen if any */
	}

		/* save original size in case have to back out */
	if (vb.pencel != NULL)
		{	/* This bracket necessary due to wierdness in copy_rectfields
		     * macro */
		copy_rectfields(vb.pencel,&osize);
		}
	else 
		osize.width = 0;

	if ((err = set_pencel_size(width,height,vl.flicent.x,vl.flicent.y)) 
		< Success)
		goto error;

	if(!undof) /* we need one */
	{
		if((err = alloc_undof(width,height)) < 0)
			goto error;
	}

	/* make sure there is enough room for a cbuf */
	if((err = pj_fli_cel_alloc_cbuf(((void *)(&cbuf)), undof)) < Success)
		goto error;
	pj_free(cbuf);
	goto reset;

error:
	close_wndo((Wndo *)vb.pencel);
	if(!restore_size && osize.width) /* attempt to put back to previous */
	{
		vb.pencel = NULL;
		restore_size = 1;
		set_penwndo_size(osize.width, osize.height); /* ignore return */
		restore_size = 0;
	}
	else /* we are attempting to go back and failed */
	{
		/* default penwndo is whole screen */
		vb.pencel = (Rcel *)&(vb.screen->wndo);
	}
reset:

	PENWNDO->doit = do_pen_tool;
	PENWNDO->ioflags = MBPEN;
	PENWNDO->cursor = &pentool_cursor;
	PENWNDO->procmouse = penwin_procmouse;
	reres_settings();
	set_render_clip(NULL);
	return(err);
}
#ifdef SLUFFED
void get_penwndo_port(Rectangle *port)

/* returns visible port rectangle of pencel relative to itself */
{
	/* get window screen port */
	crect_torect((Cliprect *)&(PENWNDO->CRECTSTART),port);
	/* offset to window */
	port->x -= PENWNDO->behind.x;
	port->y -= PENWNDO->behind.y;
}
#endif /* SLUFFED */
void set_penwndo_position()

/* called as part of rethink settings */
{
Rectangle pos;
Boolean bord_was_on;

	if(PENWNDO == NULL || PENWNDO == &vb.screen->wndo)
		return;

	if(bord_was_on = (PENWNDO->redraw != NULL))
		fliborder_off();
	pos.width = PENWNDO->width;
	pos.height = PENWNDO->height;
	pos.x = vl.flicent.x + vb.scrcent.x - (pos.width>>1);
	pos.y = vl.flicent.y + vb.scrcent.y - (pos.height>>1);
	if(PENWNDO->flags & WNDO_BACKDROP)
		bclip0xy_rect(&pos,(Rectangle *)&(vb.screen->wndo.RECTSTART));
	else
		clip_penwinrect(&pos);

	reposit_wndo(PENWNDO,&pos,NULL);
	if(bord_was_on)
		fliborder_on();
}
