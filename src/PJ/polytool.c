
/* polytool.c - Stuff to rubberband and render polygons, splines, and other
   point-listy vectory objects.  Most of this works on a global poly -
   the working_poly.  There are routines to load and save a poly file
   to/from working_poly here.  Also routines to move individual points
   in a polygon.  */

#include "errcodes.h"
#include "lstdio.h"
#include "ffile.h"
#include "jimk.h"
#include "commonst.h"
#include "inks.h"
#include "marqi.h"
#include "memory.h"
#include "poly.h"
#include "rectang.h"
#include "render.h"

Poly working_poly;
char curveflag;

/************* polygon and spline marqi and dotout drawing routines ***********/

void
poly_cline_with_render_dot(SHORT x1, SHORT y1, SHORT x2, SHORT y2, void *data)
{
	pj_cline(x1, y1, x2, y2, render_dot, data);
}

static void dot_poly(register Poly *poly, VFUNC dotout, void *dotdat)
{
register LLpoint *this;
int i;

	i = poly->pt_count;
	this = poly->clipped_list;
	while (--i >= 0)
	{
		(*dotout)(this->x,this->y,dotdat);
		this = this->next;
	}
}
marqi_polydots(Marqihdr *mh,Poly *poly)
{
	dot_poly(poly,mh->pdot,mh);
}
undo_polydots(Marqihdr *mh,Poly *poly)
{
	dot_poly(poly,undo_marqidot,mh);
}

static void cpoly(Poly *poly, void *dotout, void *data,Boolean closeit)
{
register LLpoint *this, *next;
int i;

	i = poly->pt_count;
	if(!closeit)
		--i;
	this = poly->clipped_list;
	while (--i >= 0)
	{
		next = this->next;
		pj_cline( this->x, this->y, next->x, next->y,
			   dotout, data);
		this = next;
	}
}
static marqi_open_poly(Marqihdr *mh,Poly *poly)
/* used in make poly draws all but rubba vectors */
{
	cpoly(poly,mh->pdot,mh,0);
}

static void mwpoly(Marqihdr *mh,VFUNC dotout, Poly *p, Boolean closed)
{
	if (curveflag)
		some_spline(p, dotout, mh, pj_cline, closed, 16);
	else
		cpoly(p,dotout,mh,closed);
}
void marqi_poly(Marqihdr *mh, Poly *p, Boolean closed)
{
	mwpoly(mh,mh->pdot, p, closed);
}
void undo_poly(Marqihdr *mh, Poly *p,  Boolean closed)
{
	mwpoly(mh,undo_marqidot, p, closed);
}


typedef struct dot_buffer
	{
	UBYTE *save_pixels;
	Short_xy *save_coors;
	long pt_count;
	long pt_alloc;
	UBYTE saved;
	} Dot_buffer;

dot_buffer_put(USHORT x, USHORT y, Marqihdr *mh)
{
Dot_buffer *db;
int pt_count;

db = mh->adata;
if ((pt_count = db->pt_count) < db->pt_alloc)
	{
	if (x < vb.pencel->width && y < vb.pencel->height)
		{
		db->save_pixels[pt_count] = pj__get_dot(vb.pencel,x,y);
		db->save_coors[pt_count].x = x;
		db->save_coors[pt_count].y = y;
		db->pt_count = pt_count+1;
		(*mh->pdot)(x,y,mh);
		}
	}
}

restore_dot_buffer(Marqihdr *mh)
{
Dot_buffer *db;
Short_xy *coors;
UBYTE *dots;
int count;

db = mh->adata;
count = db->pt_count;
coors = db->save_coors + count;
dots = db->save_pixels + count;
while (--count >= 0)
	{
	--dots;
	--coors;
	(*mh->putdot)(vb.pencel,*dots,coors->x,coors->y);
	}
db->pt_count = 0;
}


Errcode move_spline_segment(Marqihdr *mh, int moving_point_ix, Poly *poly)
{
Dot_buffer rs;
int closed;
Errcode err;


pj_stuff_bytes(0, &rs, sizeof(rs) );
rs.pt_alloc = 4*(vb.pencel->width+vb.pencel->height);
if (((rs.save_coors = pj_malloc(rs.pt_alloc*sizeof(Short_xy))) == NULL) ||
	((rs.save_pixels = pj_malloc(rs.pt_alloc*sizeof(UBYTE))) == NULL))
	{
	err = Err_no_memory;
	goto ERROR;
	}
mh->adata = &rs;
closed = is_closedp();
partial_spline(poly, dot_buffer_put, mh, pj_cline, closed, 16,
	moving_point_ix,0);
wait_any_input();
restore_dot_buffer(mh);
err = Success;
ERROR:
pj_gentle_free(rs.save_pixels);
pj_gentle_free(rs.save_coors);
return(err);
}


/******** end marqi routines ********/



void free_polypoints(Poly *poly)
{
register LLpoint *this, *next;
int i;

i = poly->pt_count;
this = poly->clipped_list;
while (--i >= 0)
	{
	next = this->next;
	pj_free(this);
	this = next;
	}
poly->pt_count = 0;
poly->clipped_list = NULL;
}

#ifdef SLUFFED
void free_poly(Poly *p)
{
if (p != NULL)
	{
	free_polypoints(p);
	pj_free(p);
	}
}
#endif /* SLUFFED */






LLpoint *poly_last_point(Poly *p)
{
	return(slist_el(p->clipped_list, p->pt_count-1));
}

static LLpoint *beg_this_mouse(void)
{
LLpoint *this;

if ((this = begmem(sizeof(*this))) == NULL)
	return(NULL);
this->x = icb.mx;
this->y = icb.my;
this->z = 0;
return(this);
}

Errcode render_poly(Poly *wply, Boolean filled, Boolean closed)
{
Errcode err;

	/* We don't render polys so well in line grad ink, so force line
	   gradient to horizontal gradient. */

	disable_lsp_ink();
	if((err = make_render_cashes()) >= Success)
	{
		err = csd_render_poly(wply, filled,closed);
		free_render_cashes();
	}
	enable_lsp_ink();
	return(err);
}
void poly_grad_dims(Poly *p, Boolean filled)
{
Rectangle r;
int halfsize;
int off;


	off = ((!filled || vs.color2) && vs.use_brush)?get_brush_size():0; 
	find_pminmax(p, &r);
	halfsize = off - off/2;
	r.x -= halfsize;
	r.y -= halfsize;
	r.height += off;
	r.width += off;
	set_gradrect(&r);
}

Errcode render_fill_poly(Poly *p)
{
	start_abort_atom();
	return(errend_abort_atom(filled_polygon(p,poll_render_hline,
					vb.pencel, poly_cline_with_render_dot, NULL)));
}



Errcode render_a_poly_or_spline(struct poly *poly, Boolean filled,
	Boolean closed, Boolean curved)
{
Errcode err;
int oc;


/*	off = ((!filled || vs.color2) && vs.use_brush)?get_brush_size():0; */ 

	dirties();
	if (curved)
	{
		if (filled)
			err = filled_spline(poly);
		else
			err = hollow_spline(poly,closed);
	}
	else
	{
		poly_grad_dims(poly,filled);
		if (filled)
			err = render_fill_poly(poly);
		else
			err = render_opoly(poly,closed);
	}

	if(err < Success)
		goto error;

	if(filled && vs.color2)
	{
		oc = vs.ccolor;
		vs.ccolor = vs.inks[7];
		free_render_cashes();
		make_render_cashes();
		if(curved)
			err = hollow_spline(poly,TRUE);
		else
			err = render_opoly(poly,TRUE);
		vs.ccolor = oc;
	}

error:
	return(err);
}

Errcode csd_render_poly(Poly *poly, Boolean filled, Boolean closed)
{
	return render_a_poly_or_spline(poly, filled, closed, curveflag);
}


Errcode finish_polyt(Boolean filled, Boolean closed)
{
Errcode err;

	save_poly(poly_name, &working_poly); /* if we can't save don't report */
	err = render_poly(&working_poly, filled,closed);
	free_polypoints(&working_poly);
	save_redo_poly(curveflag);
	if (vs.cycle_draw)
		cycle_redraw_ccolor();
	return(err);
}

Errcode maybe_finish_polyt(Boolean filled, Boolean closed)
{
	if (JSTHIT(MBPEN))
		return(finish_polyt(filled,closed));

	free_polypoints(&working_poly);
	return(Err_abort);
}

LLpoint *new_poly_point(Poly *poly)
{
LLpoint *this;

	if ((this = beg_this_mouse()) == NULL)
		return(NULL);
	if (poly->clipped_list == NULL)
	{
		poly->clipped_list = this->next = this;
	}
	else
	{
		poly_last_point(poly)->next = this;
		this->next = poly->clipped_list;
	}
	poly->pt_count++;
	return(this);
}

#ifdef SLUFFED
LLpoint *poly_add_point(void)
{
return(new_poly_point(&working_poly));
}
#endif /* SLUFFED */


LLpoint *start_polyt(Poly *p)
{
	free_polypoints(p);
	if (!pti_input())
		return(NULL);
	save_undo();
	return(new_poly_point(p));
}


Errcode polyf_tool(void)
{
Errcode err;

	if((err = make_poly(&working_poly, vs.closed_curve))>=Success)
		err = finish_polyt(vs.fillp,vs.closed_curve);
	return(err);
}

#ifdef SLUFFED
void mmake_path(void)
{
	hide_mp();
	make_path();
	show_mp();
}
#endif /* SLUFFED */

make_poly_loop(Poly *poly, Boolean curved, Boolean closed, LLpoint *this,
			   int color)
{
LLpoint *prev;
LLpoint *next;
Marqihdr mh;
int cur_point_ix = 0;

	cinit_marqihdr(&mh,color,color,TRUE);
	for (;;)
	{
		this->x = icb.mx;
		this->y = icb.my;
		this->z = 0;
		if (JSTHIT(MBPEN))
		{
			if ((this = new_poly_point(poly)) == NULL)
			{
				undo_poly(&mh, poly, closed);
				break;
			}
			cur_point_ix++;
			if(curved)
			{
				/* draw the bits that won't change as they move around
				a point */
				partial_spline(poly, mh.pdot, &mh,
					pj_cline, closed, 16, cur_point_ix,1);
			}
			else if(poly->pt_count >= 2) /* should always happen */
			{
				prev = slist_el(poly->clipped_list,
							poly->pt_count-2);
			}
		}
		if (JSTHIT(MBRIGHT))
		{
			undo_poly(&mh, poly, closed);
			pj_free(this);
			poly->pt_count-=1;
			if (poly->pt_count >= 1)
				poly_last_point(poly)->next
								= poly->clipped_list;
			break;
		}
		if(curved)
		{
			move_spline_segment(&mh, cur_point_ix, poly);
		}
		else
		{
			if(closed)
				next = this->next;
			else
				next = prev;
			marqi_open_poly(&mh,poly);
			for(;;)
			{
				get_rub_vertex((Short_xy *)&(prev->x),
					   (Short_xy *)&(this->x),
					   (Short_xy *)&(next->x), color);
				if(JSTHIT(MBPEN|MBRIGHT))
					break;
			}
		}
	}
}

Errcode make_poly(Poly *p, Boolean closed)
{
LLpoint *this;

	if ((this = start_polyt(p)) == NULL)
		return(Err_abort);
	make_poly_loop(p, curveflag, closed, this, vs.ccolor);
	return(Success);
}

Errcode curve_tool(void)
{
Errcode err;

	curveflag = 1;
	err = polyf_tool();
	curveflag = 0;
	return(err);
}

Errcode shapef_tool(void)
{
	if(start_polyt(&working_poly) == NULL)
		return(Err_no_memory);
	get_rub_shape(&working_poly,vs.ccolor,vs.ccolor);
	return(finish_polyt(vs.fillp,vs.closed_curve));
}

static long lround_div(long p, int q)
{
if (p > 0)
	p += q>>1;
else
	p -= q>>1;
return(p/q);
}

static long llround_div(long p,long q)
{
if (p > 0)
	p += q>>1;
else
	p -= q>>1;
return(p/q);
}

int make_sp_wpoly(Poly *poly, int x0,int y0,int rad, register int theta,
				  int points,int star,int sratio)
{
int i;
LLpoint *next;
int irad;
int itheta, temp;
register int ppoints;
int x,y;
long ellmat[2][2];
int s,c;

theta &= (TWOPI-1);
irad = (star ? (rad*sratio+50)/100 : rad);
itheta = 0;
free_polypoints(poly);
ppoints = points;
switch (star)
	{
	case WP_RPOLY:
		break;
	case WP_STAR:
		ppoints*=2;
		break;
	case WP_PETAL:	/* petals */
		ppoints *= 32;
		break;
	case WP_ELLIPSE:	/* ellipse */
		if (theta == TWOPI/4 || theta == 3*TWOPI/4)
			{
			/* swap axis and call theta 0 */
			temp = rad;
			rad = sratio;
			sratio = temp;
			theta = 0;
			}
		else if (theta == 0 || theta == TWOPI/2)
			theta = 0;
		else
			{
			s = isin(theta);
			c = icos(theta);
			ellmat[0][0] = lround_div((long)rad*s,1<<9);
			ellmat[0][1] = lround_div((long)sratio*c,1<<9);
			ellmat[1][0] = lround_div((long)rad*-c,1<<9);
			ellmat[1][1] = lround_div((long)sratio*s,1<<9);
			}
		break;
	}
poly->pt_count = ppoints;
for (i=0; i<ppoints; i++)
	{
	if ((next = begmem(sizeof(*next))) == NULL)
		return(0);
	next->next = poly->clipped_list;
	poly->clipped_list = next;
	switch (star)
		{
		case WP_RPOLY:
		case WP_STAR:
			polar((int)((long)theta + ((long)TWOPI*i+ppoints/2)/ppoints),
				(i&1 ? irad : rad),
				&next->x);
			break;
		case WP_PETAL:
			temp = intabs(icos(itheta));
			itheta += TWOPI/(32*2);
			polar((int)((long)theta + ((long)TWOPI*i+ppoints/2)/ppoints),
				irad + itmult(temp,rad-irad),
				&next->x);
			break;
		case WP_ELLIPSE:
			itheta = ((long)TWOPI*i+(ppoints>>1))/ppoints;
			if (theta == 0)
				{
				next->y = itmult(rad,icos(itheta));
				next->x = itmult(sratio,isin(itheta));
				}
			else
				{
				x = icos(itheta);
				y = isin(itheta);
				next->x = llround_div(x*ellmat[0][0]+y*ellmat[0][1],(1L<<19));
				next->y = llround_div(x*ellmat[1][0]+y*ellmat[1][1],(1L<<19));
				}
			break;
		}
	next->x += x0;
	next->y += y0;
	next->z = 0;
	}
poly_last_point(poly)->next = poly->clipped_list;
return(1);
}

polystar_loop(Poly *poly,
	int star, int dot_color, int dash_color, int x0, int y0,
	int *ptheta, int *prad)
{
int theta, rad;
Marqihdr mh;
Errcode err = Success;

cinit_marqihdr(&mh,dot_color,dash_color,TRUE);
for (;;)
	{
	rad = calc_distance(icb.mx,icb.my,x0,y0);
	theta = -arctan(icb.mx - x0, icb.my - y0);
	if (!make_sp_wpoly(poly,
		x0,y0,rad,theta,vs.star_points,star,vs.star_ratio))
		break;
	marqi_poly(&mh, poly, TRUE);
	wait_any_input();
	undo_poly(&mh, poly, TRUE);
	if (JSTHIT(MBRIGHT|KEYHIT))
		{
		err = Err_abort;
		break;
		}
	if (JSTHIT(MBPEN))
		break;
	}
*ptheta = theta;
*prad = rad;
return(err);
}


static Errcode polystartool(int star)
{
int theta, rad;
	if (!pti_input())
		return(Success);
	save_undo();
	polystar_loop(&working_poly, star, vs.ccolor, vs.ccolor, icb.mx, icb.my,
		&theta, &rad);
	return(maybe_finish_polyt(vs.fillp,TRUE));
}

Errcode rpolyf_tool(void)
{
	return(polystartool(WP_RPOLY));
}

Errcode starf_tool(void)
{
	return(polystartool(WP_STAR));
}

Errcode petlf_tool(void)
{
	return(polystartool(WP_PETAL));
}

Errcode check_poly_file(char *filename)
{
Errcode err;
Poly poly;

if ((err = read_gulp(filename, &poly, sizeof(poly))) < Success)
	return err;
if (poly.polymagic != POLYMAGIC)
	return Err_bad_magic;
return Success;
}

int ld_poly(FILE *f, Poly *poly)
{
Errcode err;
int i;
int count;
LLpoint *this;

	free_polypoints(poly);
	if((err = ffread(f, poly, sizeof(*poly))) < Success)
		goto head_error;
	if(poly->polymagic != POLYMAGIC)
	{
		err = Err_bad_magic;
		goto head_error;
	}
	count = poly->pt_count;
	poly->pt_count = 0;
	poly->clipped_list = NULL;
	for (i=0; i<count; i++)
	{
		if ((this = new_poly_point(poly)) != NULL)
		{
			if((err = ffread(f, &this->x, 3*sizeof(SHORT))) < Success)
				goto error;
		}
		else
		{
			err = Err_no_memory;
			goto error;
		}
	}
	return(Success);
error:
	free_polypoints(poly);
head_error:
	clear_struct(poly);
	return(err);
}

Errcode load_a_poly(char *name, Poly *poly)
{
Errcode err;
FILE *f;

	if((err = ffopen(name, &f, rb_str)) >= Success)
		err = ld_poly(f, poly);
	ffclose(&f);
	return(err);
}

Errcode s_poly(FILE *f, Poly *poly)
{
Errcode err;
int i;
LLpoint *pt;

	poly->polymagic = POLYMAGIC;
	pt = poly->clipped_list;
	poly->clipped_list = NULL;
	err = ffwrite(f, poly, sizeof(Poly));
	poly->clipped_list = pt;
	if(err < Success)
		goto error;
	i = poly->pt_count;
	while(--i >= 0)
	{
		if((err = ffwrite(f, &pt->x, 3*sizeof(SHORT))) < Success)
			goto error;
		pt = pt->next;
	}
	return(Success);
error:
	return(err);
}

int save_poly(char *name, Poly *poly)
{
Errcode err;
FILE *f;

	if((err = ffopen(name, &f, wb_str)) < Success)
		goto error;
	err = s_poly(f, poly);
error:
	ffclose(&f);
	if(err < Success)
		pj_delete(name);
	return(err);
}

LLpoint *closest_point(Poly *p, int x, int y, long *dsquared)
{
int i;
LLpoint *pt;
LLpoint *closest;
long closestd, curd;
long dx,dy;

pt = closest = p->clipped_list;
closestd = 0x7fffffff;
i = p->pt_count;
while (--i >= 0)
	{
	dx = x-pt->x;
	dy = y-pt->y;
	curd = dx*dx+dy*dy;
	if (curd < closestd)
		{
		closestd = curd;
		closest = pt;
		}
	pt = pt->next;
	}
*dsquared = closestd;
return(closest);
}

void pp_find_next_prev(Poly *poly, LLpoint *point,
	LLpoint **next, LLpoint **prev)
/* find point before and after a given point in poly */
{
LLpoint *pt;

pt = poly->clipped_list;
while (pt->next != point)
	pt = pt->next;
*prev = pt;
pt = pt->next;
pt = pt->next;
*next = pt;
}


int is_closedp(void)
{
extern int is_path;

if (is_path)
	return(vs.pa_closed);
else
	return(vs.closed_curve);
}

Errcode load_and_paste_poly()
{
Poly poly;
Errcode err;

	clear_struct(&poly);
	if ((err = load_a_poly(poly_name, &poly)) >= Success)
	{
		err = render_poly(&poly, vs.fillp,vs.closed_curve);
		if (vs.cycle_draw)
			cycle_redraw_ccolor();
		free_polypoints(&poly);
	}
	return(err);
}
Errcode edit_poly_file(char *name, char curve)
{
Poly poly;
Errcode err;
char ocurve;

	clear_struct(&poly);
	if((err = load_a_poly(name, &poly)) < Success)
		return(err);
	ocurve = curveflag;
	curveflag = curve;
	move_poly_points(&poly, vs.closed_curve);
	save_poly(poly_name, &poly);
	free_polypoints(&poly);
	curveflag = ocurve;
	return(err);
}

void edit_poly(void)
{
	hide_mp();
	if(load_a_poly(poly_name, &working_poly) >= 0)
	{
		move_poly_points(&working_poly, vs.closed_curve);
		finish_polyt(vs.fillp,vs.closed_curve);
	}
	show_mp();
}

void edit_curve(void)
{
	curveflag = 1;
	edit_poly();
	curveflag = 0;
}


void linkup_poly(Poly *p)
{
int i;
LLpoint *pt;

i = p->pt_count;
pt = p->clipped_list;
while (--i >= 0)
	{
	pt->next = pt+1;
	pt++;
	}
--pt;
pt->next = p->clipped_list;
}

Errcode clone_ppoints(Poly *s, Poly *d)
/* makes d's point list a clone of s's.
   Previous d point list is overwritten, but not freed. */
{
int count;
LLpoint *np,*op;

op = s->clipped_list;
count = s->pt_count;
d->clipped_list = NULL;
d->pt_count = 0;
while (--count >= 0)
	{
	if ((np = new_poly_point(d)) == NULL)
		return(Err_no_memory);
	np->x = op->x;
	np->y = op->y;
	np->z = op->z;
	op = op->next;
	}
return(Success);
}

Errcode update_poly(Poly *s, Poly *d)
/* Make d a replica of s, with it's own point list which is a
   duplicate of s's point list. */
{
free_polypoints(d);
*d = *s;
return(clone_ppoints(s,d));
}

void dotty_disp_poly(Poly *p, Boolean closed,  Pixel dit_color, Pixel dot_color)
{
Marqihdr mh;

cinit_marqihdr(&mh,dit_color,dot_color,TRUE);
marqi_poly(&mh, p, closed);
}

void offset_poly(Poly *poly, int x, int y, int z)
{
LLpoint *pt;
int count;

pt = poly->clipped_list;
count = poly->pt_count;
while (--count >= 0)
	{
	pt->x += x;
	pt->y += y;
	pt->z += z;
	pt = pt->next;
	}
}
