
/* polytool.c - Stuff to rubberband and render polygons, splines, and other
   point-listy vectory objects.  Most of this works on a global poly -
   the working_poly.  There are routines to load and save a poly file
   to/from working_poly here.  Also routines to move individual points
   in a polygon.  */

#include "jimk.h"
#include "poly.h"

/* some functions for line drawing */
extern int copydot(),marqidot(),sdot(), xdot(), render_dot(), render_brush();
extern a1bdot(), rbdot(), rbbrush();

extern cline(), render_line();

extern int pxmin, pxmax, pymin, pymax;

Poly working_poly;

char curveflag;




poly_nopoints(poly)
Poly *poly;
{
register struct llpoint *this, *next;
int i;

i = poly->pt_count;
this = poly->clipped_list;
while (--i >= 0)
	{
	next = this->next;
	freemem(this);
	this = next;
	}
poly->pt_count = 0;
poly->clipped_list = NULL;
}

#ifdef SLUFFED
free_poly(p)
Poly *p;
{
if (p != NULL)
	{
	poly_nopoints(p);
	freemem(p);
	}
}
LLpoint *clone_poly_list(LLpoint *slist, int pcount)
{
LLpoint *spoint;
LLpoint *new, *list;
int i;

if (slist == NULL)
	return(NULL);
list = NULL;
/*go allocate point-list */
i = pcount;
while (--i >= 0)
	{
	if ((new = begmem(sizeof(*new))) == NULL)
		goto NOMEM;
	new->next = list;
	list = new;
	}
/* copy in data from old point-list */
spoint = slist;
new = list;
i = pcount;
for (;;)
	{
	new->x = spoint->x;
	new->y = spoint->y;
	new->z = spoint->z;
	if (--i == 0)
		break;
	new = new->next;
	spoint = spoint->next;
	}
/* link last element in new list to 1st element */
new->next = list;
return(list);
NOMEM:
	{
	while (list)
		{
		new = list->next;
		freemem(list);
		list = new;
		}
	return(NULL);
	}
}

Poly *clone_poly(Poly *p)
{
extern void *clone_structure();
Poly *new;

if ((new = clone_structure(p, sizeof(*p))) != NULL)
	{
	if ((new->clipped_list = clone_poly_list(
		p->clipped_list, p->pt_count)) == NULL)
		{
		freemem(new);
		new = NULL;
		}
	}
return(new);
}

#endif SLUFFED

dot_poly(poly, dotout)
register Poly *poly;
Vector dotout;
{
register struct llpoint *this;
int i;

i = poly->pt_count;
this = poly->clipped_list;
while (--i >= 0)
	{
	(*dotout)(this->x,this->y);
	this = this->next;
	}
}

some_poly(poly, vector)
Poly *poly;
Vector vector;
{
register struct llpoint *this, *next;
int i;

i = poly->pt_count;
if (!poly->closed || !is_closedp())
	--i;
this = poly->clipped_list;
while (--i >= 0)
	{
	next = this->next;
	cline( this->x, this->y, next->x, next->y, 
		vector);
	this = next;
	}
}

LLpoint *
poly_last_point(p)
Poly *p;
{
LLpoint *this;
int ptcount;

this = p->clipped_list;
ptcount = p->pt_count-1;
while (--ptcount >= 0)
	this = this->next;
return(this);
}

static LLpoint *
beg_this_mouse()
{
LLpoint *this;

if ((this = begmem(sizeof(*this))) == NULL)
	return(NULL);
this->x = grid_x;
this->y = grid_y;
this->z = 0;
return(this);
}

render_poly(wply, filled)
Poly *wply;
int filled;
{
int ok;

if (make_render_cashes())
	{
	ok = csd_render_poly(wply, filled);
	free_render_cashes();
	return(ok);
	}
return(0);
} 

rado_poly(ado_s, ptcount, filled, curved)
Point *ado_s;
int ptcount;
int filled;
int curved;
{
Poly p;
LLpoint *list;
int ok;
int i;

if ((list = p.clipped_list = begmem(ptcount * sizeof(LLpoint))) != NULL)
	{
	curveflag = curved;
	i = p.pt_count = ptcount;
	while (--i >= 0)
		{
		list->x = ado_s->x;
		list->y = ado_s->y;
		list++;
		ado_s++;
		}
	linkup_poly(&p);
	p.closed = is_closedp();
	ok = render_poly(&p, filled);
	freemem(p.clipped_list);
	curveflag = 0;
	return(ok);
	}
return(0);
}

static int off, off2;

poly_grad_dims(p)
Poly *p;
{
find_pminmax(p);
render_xy(pxmin-off2,pymin-off2,pxmax+off-off2,pymax+off-off2);
}

static
csd_render_poly(wply,filled)
Poly *wply;
int filled;
{
int oc;
int ocl;
int  ok = 1;

if (!filled || vs.color2)
	off = vs.pen_width;
off2 = off -off/2;
dirties();
if (curveflag)
	{
	if (filled)
		ok = filled_spline(wply);
	else
		hollow_spline(wply);
	}
else
	{
	poly_grad_dims(wply);
	if (filled)
		ok = fill_concave(wply);
	else
		render_opoly(wply);
	}
if (ok  && filled && vs.color2)
	{
	oc = vs.ccolor;
	ocl = vs.closed_curve;
	vs.closed_curve = 1;
	vs.ccolor = vs.inks[7];
	free_render_cashes();
	make_render_cashes();
	wply->closed = 1;
	if (curveflag)
		{
		hollow_spline(wply);
		}
	else
		render_opoly(wply);
	vs.ccolor = oc;
	vs.closed_curve = ocl;
	}
return(ok);
}


finish_polyt(filled)
int filled;
{
save_working_poly();
if (vs.cycle_draw) cycle_ccolor();
render_poly(&working_poly, filled);
poly_nopoints(&working_poly);
}

maybe_finish_polyt(filled)
int filled;
{
if (PJSTDN)
	{
	finish_polyt(filled);
	}
else
	poly_nopoints(&working_poly);
}

LLpoint *
poly_add_point()
{
LLpoint *this;

if ((this = beg_this_mouse()) == NULL)
	return(NULL);
if (working_poly.clipped_list == NULL)
	{
	working_poly.clipped_list = this->next = this;
	}
else
	{
	poly_last_point(&working_poly)->next = this;
	this->next = working_poly.clipped_list;
	}
#ifdef LATER
working_poly.clipped_list = this;
#endif LATER
working_poly.pt_count++;
return(this);
}

LLpoint *
start_polyt()
{
LLpoint *this;

if (!vs.fillp)
	brushcursor = dot_pens[vs.pen_width];
if (!pti_input())
	return(NULL);
save_undo();
if ((this = poly_add_point()) == NULL)
	return(NULL);
return(this);
}


polyf_tool()
{
if (make_poly())
	finish_polyt(vs.fillp);
}

#ifdef SLUFFED
mmake_path()
{
hide_mp();
make_path();
draw_mp();
}
#endif SLUFFED


make_poly()
{
LLpoint *this;

working_poly.closed = vs.closed_curve;
if ((this = start_polyt()) == NULL)
	return(0);
clickonly = 1;
for (;;)
	{
	this->x = grid_x;
	this->y = grid_y;
	this->z = 0;
	if (PJSTDN)
		{
		if ((this = poly_add_point()) == NULL)
			goto OUT;	
		}
	if (RJSTDN)
		{
		freemem(this);
		working_poly.pt_count-=1;
		if (working_poly.pt_count >= 1)
			poly_last_point(&working_poly)->next = working_poly.clipped_list;
		break;
		}
	rub_wpoly();
	wait_input();
	undraw_wpoly();
	}
OUT:
clickonly = 0;
return(1);
}

curve_tool()
{
curveflag = 1;
polyf_tool();
curveflag = 0;
}

static int occv;

static
push_ccv()
{
occv = vs.closed_curve;
vs.closed_curve = 1;
}

static
pop_ccv()
{
vs.closed_curve = occv;
}

shapef_tool()
{
LLpoint *this, *last;
int oc;

working_poly.closed = 1;
if ((this = start_polyt()) == NULL)
	return(0);
push_ccv();
for (;;)
	{
	if (PDN)
		{
		last = this;
		if ((this = poly_add_point()) == NULL)
			goto OUT;	
		cline(last->x, last->y, this->x, this->y, 
			sdot);
		}
	else
		break;
	wait_input();
	}
OUT:
unundo();
finish_polyt(vs.fillp);
pop_ccv();
return(1);
}



static long
lround_div(p,q)
long p;
int q;
{
if (p > 0)
	p += q>>1;
else
	p -= q>>1;
return(p/q);
}

static long
llround_div(p,q)
long p,q;
{
if (p > 0)
	p += q>>1;
else
	p -= q>>1;
return(p/q);
}

make_sp_wpoly(x0,y0,rad,theta,points,star,sratio)
int x0,y0,rad,points,star,sratio;
register int theta;
{
int i;
LLpoint *next;
int irad,trad;
int itheta, temp;
register int ppoints;
int x,y;
long ellmat[2][2];
int s,c;

theta &= (TWOPI-1);
irad = (star ? (rad*sratio+50)/100 : rad);
itheta = 0;
poly_nopoints(&working_poly);
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
working_poly.pt_count = ppoints;
for (i=0; i<ppoints; i++)
	{
	if ((next = begmem(sizeof(*next))) == NULL)
		return(0);
	next->next = working_poly.clipped_list;
	working_poly.clipped_list = next;
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
poly_last_point(&working_poly)->next = working_poly.clipped_list;
working_poly.closed = (star != 3);
return(1);
}


static
polystartool(star)
int star;
{
int x0,y0;
int theta, rad;

if (!vs.fillp)
	brushcursor = dot_pens[vs.pen_width];
if (!pti_input())
	return;
save_undo();
clickonly = 1;
x0 = grid_x;
y0 = grid_y;
push_ccv();
for (;;)
	{
	some_poly(&working_poly, sdot);
	wait_input();
	some_poly(&working_poly,copydot);
	rad = calc_distance(grid_x,grid_y,x0,y0);
	theta = -arctan(grid_x - x0, grid_y - y0);
	if (!make_sp_wpoly(x0,y0,rad,theta,vs.star_points, star, vs.star_ratio))
		break;
	if (PJSTDN || RJSTDN)
		break;
	}
clickonly = 0;
maybe_finish_polyt(vs.fillp);
pop_ccv();
}

rpolyf_tool()
{
polystartool(WP_RPOLY);
}

starf_tool()
{
polystartool(WP_STAR);
}

petlf_tool()
{
polystartool(WP_PETAL);
}

static
l_working_poly(f)
Bfile *f;
{
int i;
int count;
LLpoint *this, *last;
int abort;

poly_nopoints(&working_poly);
if (bread(f, &working_poly, sizeof(working_poly)) < 
	sizeof(working_poly) )
	{
	goto ABORT;
	}
if (working_poly.polymagic != POLYMAGIC)
	{
	goto ABORT;
	}
count = working_poly.pt_count;
working_poly.pt_count = 0;
working_poly.clipped_list = NULL;
for (i=0; i<count; i++)
	{
	if ((this = poly_add_point()) != NULL)
		{
		if ((bread(f, &this->x, 3*sizeof(WORD) )) < 3*sizeof(WORD) )
			break;
		}
	}
return(1);
ABORT:
	zero_structure(&working_poly, sizeof(working_poly) );
	return(0);
}

load_working_poly()
{
return(load_poly(poly_name) );
}

load_poly(name)
char *name;
{
int res;
Bfile bf;

if ((bopen(name, &bf)) == 0)
	return(0);
res = l_working_poly(&bf);
bclose(&bf);
return(res);
}

static
s_poly(f, poly)
Bfile *f;
Poly *poly;
{
int i;
LLpoint *pt;

poly->polymagic = POLYMAGIC;
pt = poly->clipped_list;
poly->clipped_list = NULL;
if (bwrite(f, poly, sizeof(Poly) ) < sizeof(Poly) )
	return(0);
poly->clipped_list = pt;
i = poly->pt_count;
while (--i >= 0)
	{
	if (bwrite(f, &pt->x, 3*sizeof(WORD) ) < 3*sizeof(WORD) )
		return(0);
	pt = pt->next;
	}
return(1);
}

static
save_working_poly()
{
return(save_poly(poly_name, &working_poly));
}

save_poly(name,  poly)
char *name;
Poly *poly;
{
int res;
Bfile bf;

if ((bcreate(name, &bf)) == 0)
	return(0);
res = s_poly(&bf, poly);
res &= bclose(&bf);
if (res == 0)
	{
	cant_create(name);
	jdelete(name);
	}
return(res);
}

static LLpoint *
closest_point()
{
int i;
LLpoint *pt;
LLpoint *closest;
int closestd, curd;

pt = closest = working_poly.clipped_list;
closestd = 32000;
i = working_poly.pt_count;
while (--i >= 0)
	{
	if ((curd = calc_distance(grid_x, grid_y, pt->x, pt->y)) < closestd)
		{
		closestd = curd;
		closest = pt;
		}
	pt = pt->next;
	}
return(closest);
}

is_closedp()
{
extern int is_path;

if (is_path)
	return(vs.pa_closed);
else
	return(vs.closed_curve);
}

static
undraw_wpoly()
{
if (curveflag)
	some_spline(&working_poly, copydot, cline, is_closedp(), 16);
else
	some_poly(&working_poly,copydot);
}

rub_wpoly()
{
int lclosed;

if (curveflag)
	some_spline(&working_poly, sdot, cline, is_closedp(), 16);
else
	some_poly(&working_poly,sdot);
}

static
rub_wpoints()
{
LLpoint *p;
int i;
extern UBYTE circ3_cursor[];

p = working_poly.clipped_list;
i = working_poly.pt_count;
while (--i >= 0)
	{
	a1blit(16, 5, 0, 0, circ3_cursor+12, 2, p->x-8, p->y-2, render_form->p,
		BPR, vs.ccolor);
	p = p->next;
	}
}

undraw_wpoints()
{
LLpoint *p;
int i;
int x,y;

p = working_poly.clipped_list;
i = working_poly.pt_count;
while (--i >= 0)
	{
	x = p->x-2;
	y = p->y-2;
	blit8(5,5,x,y,uf.p,BPR,x,y,render_form->p,BPR);
	p = p->next;
	}
}

move_poly_points()
{
LLpoint restore;
LLpoint *lp;

working_poly.closed = vs.closed_curve;
save_undo();
for (;;)
	{
	rub_wpoly();
	rub_wpoints();
	wait_click();
	if (RJSTDN || key_hit || (lp = closest_point()) == NULL)
		{
		undraw_wpoly();
		undraw_wpoints();
		return;
		}
	copy_structure(lp, &restore, sizeof(restore) );
	for (;;)
		{
		wait_input();
		undraw_wpoly();
		undraw_wpoints();
		if (RJSTDN || key_hit)
			{
			copy_structure(&restore, lp, sizeof(*lp) );
			break;
			}
		if (PJSTDN)
			{
			break;
			}
		lp->x = grid_x;
		lp->y = grid_y;
		rub_wpoly();
		rub_wpoints();
		}
	}
}

linkup_poly(p)
Poly *p;
{
int i;
register LLpoint *pt;
LLpoint *last;

i = p->pt_count;
pt = p->clipped_list;
while (--i >= 0)
	{
	last = pt;
	pt = pt->next = norm_pointer(pt+1);
	}
last->next = p->clipped_list;
}

