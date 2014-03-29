#include "jimk.h"
#include "auto.h"
#include "commonst.h"
#include "errcodes.h"
#include "fli.h"
#include "inkdot.h"
#include "inks.h"
#include "memory.h"
#include "palmenu.h"
#include "pentools.h"
#include "poly.h"
#include "rastcall.h"
#include "redo.h"
#include "render.h"
#include "xfile.h"
#include "zoom.h"

/* stuff to deal with redo-draw */
static XFILE *rbf;

static Errcode _redo_draw(Redo_rec *r);

Errcode start_save_redo_points(void)
{
	if ((rbf = xfopen(rbf_name, wb_str)) == NULL)
		return softerr(xerrno(), "redo_save");
	return(Success);
}

void end_save_redo_points(void)
{
	if(rbf != NULL)
	{
		xfclose(rbf);
		rbf = NULL;
	}
}

Errcode save_redo_point(Pos_p *p)
{
if (xfwrite(p, 1, sizeof(*p), rbf) < sizeof(*p))
	return softerr(xerrno(), "redo_points");
return(Success);
}

Errcode save_spray_redo(Spray_redo *sr)
{
if (xfwrite(sr, 1, sizeof(*sr), rbf) < sizeof(*sr))
	return softerr(xerrno(), "redo_points");
return(Success);
}
Boolean get_spray_redo(Spray_redo *sr)
{
	return (xfread(sr, 1, sizeof(*sr), rbf) == sizeof(*sr));
}

Errcode save_redo_draw(int mode)
{
	vs.redo.type = REDO_DRAW;
	vs.redo.p.draw_p = mode;
	return(Success);
}

Errcode save_redo_gel(void)
{
	vs.redo.type = REDO_GEL;
	return(Success);
}

Errcode save_redo_spray(void)
{
	vs.redo.type = REDO_SPRAY;
	return(Success);
}

static Errcode redo_draw_get_pos(Pos_p *p, void *xfile, SHORT mode)
{
	XFILE *f = xfile;
	Errcode err;
	(void)mode;

	if((err = poll_abort()) < Success)
		return(err);
	if (xfread(p, 1, sizeof(*p), f) == sizeof(*p))
		return(Success); 
	return(Success + 1); /* no more left, but not error */
}

static Errcode redo_draw(Redo_rec *r)
{
Errcode err;

	if ((rbf = xfopen(rbf_name, rb_str)) == NULL)
		return(Err_abort);

	/* line fill ink would look funky here */
	disable_lsp_ink();
	err = dtool_loop(redo_draw_get_pos, rbf, r->p.draw_p);
	xfclose(rbf);
	enable_lsp_ink();
	return(err);
}

static Errcode redo_gel(Redo_rec *r)
{
Errcode err;
(void)r;

	if ((rbf = xfopen(rbf_name, rb_str)) == NULL)
		return(Err_abort);
	err = gel_tool_loop(redo_draw_get_pos, rbf);
	xfclose(rbf);
	return(err);
}

static Errcode redo_spray(Redo_rec *r)
{
Errcode err;
(void)r;

	if ((rbf = xfopen(rbf_name, rb_str)) == NULL)
		return(Err_abort);
	err = spray_loop(redo_draw_get_pos, rbf, TRUE);
	xfclose(rbf);
	return(err);
}

/* End stuff to redo-draw */

/* Start stuff for redo separate */
Errcode save_redo_sep(Sep_p *sep)
{
	vs.redo.type = REDO_SEP;
	vs.redo.p.sep_p = *sep;
	return(write_gulp(rbf_name, sep->ctable, (long)sep->ccount));
}

static Errcode redo_sep(Redo_rec *r)
{
Sep_p *sep;
Errcode err;

	sep = &r->p.sep_p;
	if ((sep->ctable = pj_malloc(sep->ccount)) == NULL)
		return(Err_no_memory);
	if ((err = read_gulp(rbf_name, sep->ctable, (long)sep->ccount)) 
						< Success)
	{
		return(err);
	}
	return(do_sep_redo(sep));
}
/* end redo sep stuff */

static Errcode redo_line(Redo_rec *r)
{
Errcode err;
#define xys r->p.line_p

	set_full_gradrect();
	if((err = make_render_cashes()) < Success)
		return(err);
	err = render_1_line(xys[0].x,xys[0].y,xys[1].x,xys[1].y);
	free_render_cashes();
	return(err);
#undef xys
}

static Errcode redo_flood(Redo_rec *r)
{
#define fpt r->p.flood_p
	return(flood(fpt[1].x,fpt[1].y,pj_get_dot(vb.pencel,fpt[0].x, fpt[0].y)));
#undef fpt
}

static Errcode redo_fill(Redo_rec *r)
{
	return(fill(r->p.fill_p.x, r->p.fill_p.y));
}


static Errcode redo_circle(Redo_rec *r)
{
#define c (&r->p.circle_p)
Errcode err;
SHORT ocolor;

	if(!vs.fillp)
		return render_circle((Raster *)vb.pencel,
				c->center.x, c->center.y, c->diam);

	err = render_disk((Raster *)vb.pencel, c->center.x, c->center.y, c->diam);
	if (err < Success)
		return(err);

	if (vs.color2)
	{
		ocolor = vs.ccolor;
		vs.ccolor = vs.inks[7];
		err = render_circle((Raster *)vb.pencel,
				c->center.x, c->center.y, c->diam);
		vs.ccolor = ocolor;
	}
	return(err);

#undef c
}

#ifdef WITH_POCO
Errcode rend_circ(int x, int y, int radius)
{
Redo_rec rr;
#define ci (rr.p.circle_p)
	rr.type = REDO_CIRCLE;
	ci.center.x = x;
	ci.center.y = y;
	ci.diam = (radius<<1)+1;
	return(_redo_draw(&rr));
#undef ci
}
#endif /* WITH_POCO */

static Errcode redo_box(Redo_rec *r)
{
	return(render_beveled_box(&r->p.rect_p, vs.box_bevel, vs.fillp));
}

static Errcode redo_text(Redo_rec *r)
{
	(void)r;

	if(pj_exists(text_name))
		return(softerr(load_and_paste_text(text_name), "redo_text"));
	return(Err_abort);
}
static Errcode redo_edge(Redo_rec *r)
{
	return(edge1(pj_get_dot(vb.pencel,r->p.edge_p.x,r->p.edge_p.y)));
}
static Errcode redo_poly(Redo_rec *r)
{
Errcode err;
int scf;

	if(!pj_exists(poly_name))
		return(Err_abort);
	scf = curveflag;
	curveflag = r->p.poly_p.curve;
	err = softerr(load_and_paste_poly(poly_name), "redo_poly");
	curveflag = scf;
	return(err);
}

static Errcode redo_spiral(Redo_rec *r)
{
Errcode err;
SHORT oclosed;
SHORT ofill;

	oclosed = vs.closed_curve;
	ofill = vs.fillp;
	vs.closed_curve = 0;
	vs.fillp = 0;
	err = redo_poly(r);
	vs.closed_curve = oclosed;
	vs.fillp = ofill;
	save_redo_spiral();		/* edit_poly does a save_redo_poly, so need this */
	return(err);
}

static Errcode redo_move(Redo_rec *r)
{
Errcode err;
Rcel *clipcel;
Tcolxldat xld;
Pixel ctable[COLORS];

#define m (&r->p.move_p)

	if((err = clip_celrect(vb.pencel, &m->orig, &clipcel)) < 0)
		return(err);
	if(m->clear_move_out)
	{
		pj_set_rect(vb.pencel,vs.inks[0],m->orig.x,m->orig.y,
								m->orig.width,m->orig.height);
	}
	clipcel->x = m->new.x;
	clipcel->y = m->new.y;

	xld.tcolor = vs.inks[0];
	xld.xlat = NULL;
	if (vs.render_one_color)
		{
		xld.xlat = ctable;
		make_one_color_ctable(ctable, xld.tcolor);
		}
	(*(get_celblit(vs.render_one_color)))(clipcel, 0, 0, vb.pencel, 
					   clipcel->x, clipcel->y, 
					   clipcel->width, clipcel->height,&xld, undof);
	zoom_cel(clipcel);
	if(m->clear_move_out)
	{
		do_leftbehind(m->orig.x,m->orig.y,
					  clipcel->x,clipcel->y, m->orig.width,m->orig.height,
					  (do_leftbehind_func)rect_zoom_it);
	}
	pj_rcel_free(clipcel);
	return(Success);
#undef m
}
static Errcode redo_edit_text(Redo_rec *r)
{
	(void)r;

	qpwtitles(FALSE);
	return(Success);
}
static Errcode redo_edit_poly(Redo_rec *r)
{
	return(softerr(edit_poly_file(poly_name, r->p.poly_p.curve),"redo_poly"));
}
static Errcode _redo_edit(Redo_rec *r)

/* Performs any "pre edit" for a redo record type returns Success + 1
 * if the menus were hidden and an edit was performed, Success if no
 * edit was done */
{
Errcode err = Success;
Errcode (*doedit)();

	switch(r->type)
	{
		case REDO_TEXT:
			doedit = redo_edit_text;
			break;
		case REDO_POLY:
			doedit = redo_edit_poly;
			break;
		default:
			return(Success);
	}

	hide_mp();
	if((err = (*doedit)(r)) < Success)
	{
		show_mp();
		return(err);
	}
	return(Success + 1);
}

static Errcode _redo_draw(Redo_rec *r)

/* actually draws a redo record onto the screen */
{
Errcode err;
Errcode (*rfunc)(Redo_rec *r);

	switch (r->type)
	{
		case REDO_NONE:
			return(Err_abort);
		case REDO_BOX:
			rfunc = redo_box;
			break;
		case REDO_CIRCLE:
			rfunc = redo_circle;
			break;
		case REDO_TEXT:
			rfunc = redo_text;
			break;
		case REDO_POLY:
			rfunc = redo_poly;
			break;
		case REDO_FILL:
			rfunc = redo_fill;
			break;
		case REDO_FLOOD:
			rfunc = redo_flood;
			break;
		case REDO_EDGE:
			rfunc = redo_edge;
			break;
		case REDO_LINE:
			rfunc = redo_line;
			break;
		case REDO_SPIRAL:
			rfunc = redo_spiral;
			break;
		case REDO_DRAW:
			rfunc = redo_draw;
			goto no_ccycle;
		case REDO_MOVE:
			rfunc = redo_move;
			if (!vs.render_one_color)
				goto no_ccycle;
			break;
		case REDO_GEL:
			rfunc = redo_gel;
			goto no_ccycle;
		case REDO_SPRAY:
			rfunc = redo_spray;
			goto no_ccycle;
		case REDO_SEP:
			rfunc = redo_sep;
			goto no_ccycle;
		default:
			return(errline(Err_bad_input, "Invalid redo type %d", r->type));
	}

	if((err = rfunc(r)) >= Success)
	{
		if (vs.cycle_draw)
			cycle_redraw_ccolor();
	}
	return(err);

no_ccycle:
	return(rfunc(r));
}

static Errcode
auto_redo_draw(void *r, int ix, int intween, int scale, Autoarg *aa)
{
	(void)ix;
	(void)intween;
	(void)scale;
	(void)aa;

	return _redo_draw(r);
}

static Boolean redo_exists(void)
{
	return(vs.redo.type != REDO_NONE);
}

static Errcode redo(void)
/* saves undo first does redo and sets dirties flag */
{
Errcode err;
	save_undo();
	err = _redo_draw(&vs.redo);
	dirties();
	return(err);
}

void do_auto_redo(Boolean edit)

/* this is what's called from the menu or keyboard redo button */
{
Errcode edit_err;
USHORT flags;
(void)edit;

	if(!redo_exists())
		return;
	if((edit_err = _redo_edit(&vs.redo)) < Success)
		return;

	if(vs.multi)
	{
		flags = AUTO_PUSHINKS|AUTO_HIDEMP|AUTO_UNZOOM;
		if(vl.ink->needs & INK_NEEDS_CEL)
			flags |= AUTO_USESCEL;
		go_autodraw(auto_redo_draw, &vs.redo, flags);
	}
	else
		redo();

	if(edit_err == (Success + 1))
		show_mp();
}

Errcode save_redo_box(Rectangle *r)
{
	vs.redo.p.rect_p = *r;
	vs.redo.type = REDO_BOX;
	_redo_draw(&vs.redo);
	return(Success);
}

Errcode save_redo_circle(Circle_p *cp)
{
	vs.redo.p.circle_p = *cp;
	vs.redo.type = REDO_CIRCLE;
	return(_redo_draw(&vs.redo));
}

Errcode save_redo_text(void)
{
	vs.redo.type = REDO_TEXT;
	return(Success);
}

Errcode save_redo_poly(char curve)
{
	vs.redo.type = REDO_POLY;
	vs.redo.p.poly_p.curve = curve;
	return(Success);
}

Errcode save_redo_fill(Short_xy *p)
{
	vs.redo.type = REDO_FILL;
	vs.redo.p.fill_p = *p;
	return(redo());
}

Errcode save_redo_flood(Short_xy p[2])
{
	vs.redo.type = REDO_FLOOD;
	pj_copy_bytes(p, &vs.redo.p.flood_p, sizeof(vs.redo.p.flood_p) );
	return(redo());
}

Errcode save_redo_edge(Short_xy *p)
{
	save_undo();
	vs.redo.type = REDO_EDGE;
	vs.redo.p.edge_p = *p;
	return(redo());
}

Errcode save_redo_line(Short_xy *xys[2])
{
	vs.redo.type = REDO_LINE;
	pj_copy_bytes(xys, &vs.redo.p.line_p, sizeof(vs.redo.p.line_p));
	return(_redo_draw(&vs.redo));
}

Errcode save_redo_spiral(void)
/* Note this is called after save_poly_redo() */
{
	vs.redo.type = REDO_SPIRAL;
	return(Success);
}

Errcode save_redo_move(Move_p *m)
{
	vs.redo.type = REDO_MOVE;
	vs.redo.p.move_p = *m;
	return(_redo_draw(&vs.redo));
}
void clear_redo(void)
{
	vs.redo.type = REDO_NONE;
	pj_delete(rbf_name);
}


