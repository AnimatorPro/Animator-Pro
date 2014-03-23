/* pentools.c - routines that recieve input when the cursor is over the
   drawing screen (and not in a sub-menu).  */

#include "jimk.h"
#include "broadcas.h"
#include "brush.h"
#include "errcodes.h"
#include "fli.h"
#include "inkdot.h"
#include "rastcurs.h"
#include "redo.h"
#include "render.h"
#include "util.h"

static void save_thik_line_undo(SHORT y1, SHORT y2, SHORT brushsize);
static Errcode dtool(int mode);

static void hide_brush(void)
{
	vl.hide_brush = 1;
}

static void show_brush(void)
{
	vl.hide_brush = 0;
}

static int pticheck(void *dat)
{
Wndo *ckwndo;
(void)dat;

	check_top_wndo_pos();		/* move top window if necessary. */
	if(JSTHIT(KEYHIT|MBRIGHT))
		return(1);

	if(icb.mset.wndoid == ZOOM_WNDOID)
	{
		if(!(curs_in_zoombox()))
			return(1);
		ckwndo = vl.zoomwndo;
	}
	else
		ckwndo = (Wndo *)(vb.pencel);

	if(!wndo_dot_visible(ckwndo,icb.sx - ckwndo->behind.x, 
						        icb.sy - ckwndo->behind.y ))
	{
		return(1);
	}
	return(0);
}
Boolean tti_input(void)
/* pen tool initial input  -  basically checks if you've clicked over
   another window or over current drawing window */
{
int ret;

	if(!JSTHIT(MBPEN))
	{
		display_cursor();
		ret = anim_wait_input(MBPEN,ANY_INPUT,-1,pticheck);
		undisplay_cursor();
		if(ret)
		{
			reuse_input();
			return(FALSE);
		}
		return(TRUE);
	}
	return(!pticheck(NULL));
}
Boolean pti_input(void)
/* pen tool initial input  -  basically checks if you've clicked over
   another window or over current drawing window and sets dirties flag if
   you did */
{
int ret;

if ((ret = tti_input()) != FALSE)
	dirties();
return(ret);
}
int do_pen_tool(Wndo *w)
/* do it function for a pentool window */
{
Errcode err;

	if(vl.ptool)
	{
		if(((err = (*(vl.ptool->doptool))(vl.ptool,w)) < Success)
		    && err != Err_abort)
		{
			softerr(err,"!%s", "tool_fail", vl.ptool->ot.name);
			clear_redo();
		}
	}
	return(1); 
}
void do_pentool_once(Pentool *ptool)

/* usually from a pulldown menu replaces current pen tool, waits for a 
 * pen down then executes pen tool until it exits. Restores
 * the original pen tool on return.  Reports errors from tool */
{
Pentool *optool;
Wndo *w;

	optool = vl.ptool;
	if(set_curptool(ptool) < Success)
		goto done;
	w = wait_wndo_input(ANY_CLICK);
	if(!pticheck(NULL))
		do_pen_tool(w);
done:
	set_curptool(optool);
}

Errcode box_tool(Pentool *pt)
{
Errcode err;
Rectangle rect;
(void)pt;

	if (!pti_input())
		return(Success);
	if((err = get_rub_rect(&rect)) < 0)
		goto error;
	save_undo();
	err = save_redo_box(&rect);
error:
	return(err);
}

Errcode fill_tool(Pentool *pt)
{
Short_xy fpt;
(void)pt;

	if (!pti_input())
		return(Success);
	fpt.x = icb.mx;
	fpt.y = icb.my;
	return(save_redo_fill(&fpt));
}

Errcode flood_tool(void)
{
Errcode err;
Short_xy fpt[2];

	err = Success;
	if (!pti_input())
		goto done;
	fpt[0].x = icb.mx;
	fpt[0].y = icb.my;
	vl.ptool->cursor = &fill_cursor.hdr;
	wait_click();
	if(JSTHIT(MBPEN))
	{
		fpt[1].x = icb.mx;
		fpt[1].y = icb.my;
		err = save_redo_flood(fpt);
	}
	vl.ptool->cursor = &pick_cursor.hdr;
done:
	return(err);
}

Errcode edge_tool(void)
{
Short_xy fpt;

	if (!pti_input())
		return(Success);
	fpt.x = icb.mx;
	fpt.y = icb.my;
	return(save_redo_edge(&fpt));
}


Errcode drizl_tool(void)
{
Errcode err;

	disable_lsp_ink();
	err = dtool(DT_DRIZZLE);
	enable_lsp_ink();
	return(err);
}

Errcode streak_tool(void)
{
	return(dtool(DT_STREAK));
}

Errcode draw_tool(void)
{
Errcode err;

	/* line fill ink would look funky here */
	disable_lsp_ink();
	err = dtool(DT_DRAW);
	enable_lsp_ink();
	return(err);
}

Errcode dtool_input(Pos_p *p, void *dummy, SHORT mode)
/* Get input from mouse/digitizer for draw tool loop */
{
Errcode err;
(void)dummy;

	switch (mode)
	{
		case DT_DRAW:
			wait_input(ANY_INPUT);
			break;
		case DT_STREAK:
		case DT_DRIZZLE:
			vsync_wait_input(ANY_INPUT,1);
			break;
		case DT_GEL:
			check_input(ANY_INPUT);
			break;
		case DT_SPRAY:
			vsync_wait_input(ANY_INPUT,2);
			break;
	}
	if(!ISDOWN(MBPEN))
		return(Success + 1);
	p->x = icb.mx;
	p->y = icb.my;
	p->pressure = icb.pressure;
	if((err = save_redo_point(p)) < Success)
		return(err);
	return(Success);
}

Errcode dtool_loop( Errcode (*get_posp)(Pos_p *pp, void *idata, SHORT mode),
					void *idata, SHORT mode)
{
#define DL 3
#define DLMAX 16
#define DLHI (DLMAX*DL)
SHORT delt,i,j;
SHORT delts[DL];
SHORT around;
SHORT bsize;
SHORT obsize, pen_width;
SHORT drx,dry;
Pos_p rp, op;
Boolean first = TRUE;
Errcode err;

	obsize = get_brush_size();
	pen_width = vs.use_brush?obsize:0;

	set_full_gradrect();
	if ((err = make_render_cashes()) < Success)
		goto errout;
	if ((err = start_line_undo()) < Success)
		goto errout;
	/* deal with initializing speed sampling buffer for drizzle only */

	for (i=0; i<DL; i++)
		delts[i] = DLMAX;

	/* some misc counters */
	i=0;
	around = 0;

	hide_brush();

	for (;;)
	{
		if((err = get_posp(&rp, idata, mode)) != Success)
			break;
		if (first)
		{
			/* set up drizzle last x/y */
			drx = rp.x;
			dry = rp.y;
			op = rp;
			first = FALSE;
		}
		switch (mode)
		{
			case DT_STREAK:	
				save_undo_brush(rp.y);
				render_brush(rp.x,rp.y);
				break;
			case DT_DRAW:
				save_thik_line_undo(op.y, rp.y, pen_width);
				if((err = render_line(op.x,op.y,rp.x,rp.y)) < Success)
					goto draw_error;
				break;
			case DT_DRIZZLE:
				if (around <= 0)	/* only do it every 4th time around */
				{
				delt = 0;
				for (j=0; j<DL; j++)
					delt += delts[j];

				if (delt >= DLHI)
					bsize = 0;
				else
					bsize = (DLHI/2 + (DLHI-delt)*pen_width)/(DLHI);

				set_brush_size(bsize);

				delts[i] = intabs(drx-rp.x) + intabs(dry-rp.y);
				drx = rp.x;
				dry = rp.y;
				i++;
				if (i >= DL)
					i = 0;
				around = 4;
				}
			save_thik_line_undo(op.y, rp.y, bsize);
			if((err = render_line(op.x,op.y,rp.x,rp.y)) < Success)
				goto draw_error;
			--around;
			break;
		}
		if (vs.cycle_draw) cycle_ccolor();
	op = rp;
	}

draw_error:
	show_brush();
errout:
	if(vs.cycle_draw)
		do_color_redraw(NEW_CCOLOR);
	end_line_undo();
	free_render_cashes();
	set_brush_size(obsize);
	return(err);
#undef DL
#undef DLMAX
#undef DLHI
}

static Errcode dtool(int mode)
/* wierd tool that makes line thinner the faster you go */
{
Errcode err;

	for (;;)
	{
		if (!pti_input())
			return(Success);

		reuse_input();		/* put 1st point back in input stream */
		if((err = start_save_redo_points()) >= Success)
			err = dtool_loop(dtool_input, NULL, mode);
		end_save_redo_points();
		if(err < Success)
			break;
		save_redo_draw(mode);
	}
	if(err < Success)
		check_input(ANY_INPUT); /* cause of reuse_input() */
	return(err);
}

/************** Start of line-at-a-time undo saver */
static UBYTE *ychanged;

Errcode start_line_undo()
{
	pj_cmap_copy(vb.pencel->cmap,undof->cmap);
	if ((ychanged = pj_zalloc(vb.pencel->height)) == NULL)
		return(Err_no_memory);
	return(Success);
}

void end_line_undo()
{
int y;

	if(!ychanged)
		return;

	y = vb.pencel->height;
	while (--y >= 0)
	{
		if (!(ychanged[y]))
			pj_blitrect(vb.pencel,0,y,
				 	 undof, 0, y, undof->width, 1);
	}
	pj_freez(&ychanged);
}


save_line_undo(Coor y)
{
	if (y >= 0 && y < vb.pencel->height)
	{
		if (!(ychanged[y]))
		{
			pj_blitrect(vb.pencel,0,y,
				 	 undof, 0, y, undof->width, 1);
			ychanged[y] = TRUE;
		}
	}
}

void save_lines_undo(Ucoor start, int count)
{
while (--count >= 0)
	save_line_undo(start++);
}

static void save_thik_line_undo(SHORT y1, SHORT y2, SHORT brushsize)
{
SHORT height;

height = y2 - y1;
if (height < 0)
	{
	height = -height;
	y1 = y2;
	}
brushsize += 1;
brushsize >>= 1;
save_lines_undo(y1-brushsize, height+(brushsize<<1)+1 );
}

/************** End of line-at-a-time undo saver */
Errcode spray_loop( Errcode (*get_posp)(Pos_p *pp, void *idata, SHORT mode),
					void *idata, Boolean redoing)
{
Errcode err;
LONG time_max;
SHORT roff=0;
int i;
Pos_p rp;
short xy[2];
short spread;
Boolean check_time;
Boolean pressure_sensitive = is_pressure();
Spray_redo sr;

	pj_srandom(1); /* make it repeatable... */
	set_full_gradrect();
	if((err = make_render_cashes()) < 0)
		return(err);
	if ((err = start_line_undo()) < Success)
	{
		free_render_cashes();
		return(err);
	}
	hide_brush();

	/* get all the speed we can if spraying dots */
	check_time = (!redoing) && vs.use_brush && (get_brush_size() > 2);

	for (;;)
	{
		if((err = get_posp(&rp, idata, DT_SPRAY)) != Success)
			break;

		if (pressure_sensitive)
		{
			i = ((vs.air_speed*rp.pressure)>>8);
			if (i < 1)
				i = 1;
			spread = (vs.air_spread>>1) + ((vs.air_spread*rp.pressure)>>9);
			if (spread < 1)
				spread = 1;
		}
		else
		{
			i = vs.air_speed;
			spread = vs.air_spread;
		}

		if(redoing)
		{
			if(get_spray_redo(&sr))
				i = sr.count;
			else
				i = 0;
		}
		else
		{
			sr.count = i;
			time_max = pj_clock_1000() + 30;
		}

		while(--i >= 0)
		{
			polar(pj_random() + roff++, pj_random() % spread, xy);
			xy[0]+=rp.x;
			xy[1]+= rp.y;
			save_undo_brush(xy[1]);
			render_brush(xy[0], xy[1]);
			if (vs.cycle_draw) cycle_ccolor();
			if(check_time && time_max <= pj_clock_1000())
			{
				sr.count -= i;
				break;
			}
		}

		if(!redoing)
			save_spray_redo(&sr);
	}


	show_brush();
	end_line_undo();
	free_render_cashes();
	if(vs.cycle_draw)
		do_color_redraw(NEW_CCOLOR);
	return(err);
}

Errcode spray_tool(void)
{
Errcode err;

	for (;;)
	{
		if (!pti_input())
			return(Success);
		reuse_input();
		if((err = start_save_redo_points()) >= Success)
		{
			err = spray_loop(dtool_input, NULL, FALSE);
			end_save_redo_points();
		}
		save_redo_spray();
		if(err < Success)
		{
			check_input(ANY_INPUT); /* because of reuse input */
			break;
		}
	}
	return(err);
}


Errcode circle_tool(void)
{
Errcode err;
Circle_p circp;

	if (!pti_input())
		return(Success);
	if((err = get_rub_circle(&circp.center,&circp.diam,vs.ccolor)) < 0)
		goto error;
	save_undo();
	save_redo_circle(&circp);
error:
	return(err);
}

Errcode line_tool(void)
{
Errcode err;
Short_xy xys[2];

	if (!pti_input())
		return(Success);
	if((err = get_rub_line(xys)) < 0)
		goto error;
	save_undo();
	save_redo_line(xys);
error:
	return(err);
}
void move_or_copy_tool(Wndo *wndo,Pentool *pt, Boolean clear_move_out)
/* The move tool.  User clips a cel of und area after saving it
   and plops cel down in new position. and clears the old position */

{
Errcode err;
Rcel *clipcel;
Move_p mop;
(void)wndo;
(void)pt;

if (!pti_input())
	return;
hide_mp();

if((err = get_rub_rect(&(mop.orig))) < 0)
	return;

if((err = clip_celrect(vb.pencel, &mop.orig, &clipcel)) < 0)
	goto error;

save_undo();
if(move_rcel(clipcel,FALSE,vs.render_one_color) >= 0)
	{
	mop.new.x = clipcel->x;
	mop.new.y = clipcel->y;
	mop.clear_move_out = clear_move_out;
	save_redo_move(&mop);
	}
error:
show_mp();
pj_rcel_free(clipcel);
}

void move_tool(Wndo *wndo,Pentool *pt)
{
	move_or_copy_tool(wndo, pt, TRUE);
}

void copy_tool(Wndo *wndo,Pentool *pt)
{
	move_or_copy_tool(wndo, pt, FALSE);
}
