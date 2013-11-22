
/*	a3d.c - This file contains most of the code for the optics section. */

#include "jimk.h"
#include "fli.h"
#include "flicmenu.h"
#include "a3d.h"
#include "inks.h"
#include "poly.h"
#include "commonst.h"
#include "a3d.str"

extern char curveflag;	/* draw points as polygon or curve? */
extern int is_path;		/* Use path or curve tension cont. bias? */


extern Flicmenu a3d_menu;	/* Optics panel menu */
extern Vertex rot_theta;	/* guy that gets directly dinked by xyz sliders */
extern Poly working_poly;	/* Where polygons always get loaded first */

extern Pull root_pull;		/* Base of pull-down tree */
extern Pull presets_pull;	/* 1st branch of optics pull-downs */
extern WORD got_path;		/* a3ddat.c lets us know here if there's a path */
extern char inspin;			/* a3ddat.c's flag if spin sub-panel is up */


/* Variables to hold our graphic element (source for optics) */
static Vcel *ado_cel;					/* if it's a raster element */
static Poly ado_spline1, ado_spline2;	/* if it's a vector element */

static Poly ado_path_poly;	/* a place for the path in RAM */

static Vertex *ado_vpoly;		/* 3D element point list */
static Point *ado_dpoly;		/* 2D ready to render point list */
static int	ado_ptcount;		/* How many points in above lists */

/* is it a vector element or a raster element? */
static
is_vector()
{
return (vs.ado_source == OPS_SPLINE || vs.ado_source == OPS_POLY);
}

int imax(int a, int b)
{
if (a > b)
	return(a);
else
	return(b);
}

/* Get RAM for all the buffers we need for the source side of
   the graphic element and also the path if there is one */
static
make_ado_poly()
{
char *name1, *name2;

if (is_vector())
	{
	if (jexists(poly1_name) && jexists(poly2_name))
		{
		name2 = poly1_name;
		name1 = poly2_name;
		}
	else
		{
		name1 = name2 = poly_name;
		}
	if (!load_poly(name1))
		goto BADOUT;
	copy_structure(&working_poly, &ado_spline2, sizeof(ado_spline2) );
	zero_structure(&working_poly, sizeof(working_poly) );
	if (!load_poly(name2) )
		goto BADOUT;
	copy_structure(&working_poly, &ado_spline1, sizeof(ado_spline1) );
	zero_structure(&working_poly, sizeof(working_poly) );
	ado_ptcount = imax(ado_spline1.pt_count,ado_spline2.pt_count);
	}
else
	ado_ptcount = 4;
if ((ado_vpoly = begmem( sizeof(*ado_vpoly) * ado_ptcount)) == NULL)
	goto BADOUT;
if ((ado_dpoly = begmem( sizeof( *ado_dpoly) * ado_ptcount)) == NULL)
	goto BADOUT;
if (got_path)
	{
	if (!load_poly(ppoly_name) )
		goto BADOUT;
	copy_structure(&working_poly, &ado_path_poly, sizeof(ado_path_poly) );
	zero_structure(&working_poly, sizeof(working_poly) );
	}
return(1);
BADOUT:
free_ado_poly();
return(0);
}

/* Free up everything make_ado_poly() above took off the heap */
static
free_ado_poly()
{
gentle_freemem(ado_vpoly);
ado_vpoly = NULL;
gentle_freemem(ado_dpoly);
ado_dpoly = NULL;
poly_nopoints(&ado_spline1);
poly_nopoints(&ado_spline2);
poly_nopoints(&ado_path_poly);
}


/* 2 dimensional rotation */
static
partial_rot(theta, xx, yy)
WORD theta;
WORD *xx, *yy;	
{
WORD s, c;
WORD x, y;

x = *xx;
y = *yy;

s = isin(theta);
c = icos(theta);

*xx = itmult(x,c) + itmult(y,s);
*yy = itmult(y,c) + itmult(x,-s);
}

/* Do the yaw, pitch, and roll rotation to a 3-d point relative to
   wherever they've twisted the axis.  */
static
act_rotate( point, op, scale)
register struct vertex *point;
register struct ado_setting *op;
WORD scale;
{
register WORD theta;
register WORD s,c;
WORD x,y,z;

point->x -= op->spin_center.x;
point->y -= op->spin_center.y;
point->z -= op->spin_center.z;
theta =  op->itheta1;
if (theta)
	{
	partial_rot(theta, &point->x, &point->y);
	}
theta =  op->itheta2;
if (theta)
	{
	partial_rot(theta, &point->y, &point->z);
	}
theta = op->spin_theta.x;
if (theta)
	{
	theta = itmult(theta, scale);
	partial_rot(theta, &point->x, &point->z);
	}
theta = op->spin_theta.y;
if (theta)
	{
	theta = itmult(theta, scale);
	partial_rot(theta, &point->y, &point->z);
	}
theta =  op->spin_theta.z;
if (theta)
	{
	theta = itmult(theta, scale);
	partial_rot(theta, &point->x, &point->y);
	}
theta =  -op->itheta2;
if (theta)
	{
	partial_rot(theta, &point->y, &point->z);
	}
theta =  -op->itheta1;
if (theta)
	{
	partial_rot(theta, &point->x, &point->y);
	}
point->x += op->spin_center.x;
point->y += op->spin_center.y;
point->z += op->spin_center.z;
}

/* Do the x, y, and both scaling to a single 3-d point */
static
act_size(point, op, scale)
register struct vertex *point;
register struct ado_setting *op;
WORD scale;
{
int dif1, dif2;

dif1 = point->x - op->size_center.x;
dif2 = sscale_by(dif1, op->xp, op->xq);
dif2 = sscale_by(dif2, op->bp, op->bq);
point->x += itmult(dif2-dif1, scale);

dif1 = point->y - op->size_center.y;
dif2 = sscale_by(dif1, op->yp, op->yq);
dif2 = sscale_by(dif2, op->bp, op->bq);
point->y += itmult(dif2-dif1, scale);
}

/* Do the move (translation) part of optics transform to 1 point */
static
act_move(p, op, scale)
WORD *p, *op, scale;
{
int i;

i = 3;
while (--i >= 0)
	*p++ += itmult(*op++, scale);
}

/* build a rectangular polygon fitting a cel */
static
sq_vpoly(c, dest)
Vcel *c;
register WORD *dest;
{
int x,y,w,h;

x = c->x;
y = c->y;
w = c->w;
h = c->h;

w += x-1;
h += y-1;
*dest++ = x;
*dest++ = y;
*dest++ = 0;

*dest++ = x;
*dest++ = h;
*dest++ = 0;

*dest++ = w;
*dest++ = h;
*dest++ = 0;

*dest++ = w;
*dest++ = y;
*dest++ = 0;
}

/* Give our fixed point multiply routine a better name */
#define scale_mult itmult

/* Put position along path defined by poly into delta_array */
static
calc_path_pos(poly, delta_array, scale, closed)
Poly *poly;
Vertex *delta_array;
int scale;
int closed;
{
#define TSHIFT 4
WORD samp0, little_scale;
LLpoint *samp_val0, *samp_val1;
LLpoint *pt1;
WORD samples, t0samp;
int i;

samples = poly->pt_count - 1 + closed;
if (samples < 1)
	{
	delta_array->x = delta_array->y = delta_array->z = 0;
	return;
	}
if (scale == SCALE_ONE)
	{
	samp0 = samples - 1;
	little_scale = SCALE_ONE;
	}
else
	{
	samp0 = (long)scale * samples / SCALE_ONE;
	t0samp = (long)samp0 * SCALE_ONE / samples;
	little_scale = (scale - t0samp) * samples;
	}
pt1 = samp_val0 = poly->clipped_list;
i = samp0;
while (--i >= 0)
	samp_val0 = samp_val0->next;
samp_val1 = samp_val0->next;

delta_array->x = ((scale_mult( samp_val1->x<<TSHIFT, little_scale)
	+ scale_mult(samp_val0->x<<TSHIFT, SCALE_ONE - little_scale)
	+TSHIFT/2)>>TSHIFT)-pt1->x;
delta_array->y = ((scale_mult( samp_val1->y<<TSHIFT, little_scale)
	+ scale_mult(samp_val0->y<<TSHIFT, SCALE_ONE - little_scale)
	+TSHIFT/2)>>TSHIFT)-pt1->y;
delta_array->z = ((scale_mult( samp_val1->z<<TSHIFT, little_scale)
	+ scale_mult(samp_val0->z<<TSHIFT, SCALE_ONE - little_scale)
	+TSHIFT/2)>>TSHIFT)-pt1->z;
#undef TSHIFT
}


/* Do one full optics transformation to a list of points */
static
move_vpoly(s,d,count,op,scale,path)
Vertex *s, *d;	/* It's ok for these to point to same list */
int count;		/* point count */
struct ado_setting *op;		/* the transformation */
int scale;	/* How far into this tranformation?  0 to SCALE_ONE */
int path;	/* We got a path to cope with too? */
{
Vertex path_point;
Poly *pp;
Poly sp_poly;
int ok;

if (path)
	{
	if (vs.ado_path == 0)	/* splining... */
		{
		is_path = 1;	/* tell spline to use path tension cont bias */
		ok = make_sp_poly(&ado_path_poly, &sp_poly, vs.pa_closed, 16);
		is_path = 0;
		if (!ok)
			return(0);
		calc_path_pos(&sp_poly, &path_point, scale, vs.pa_closed);
		freemem(sp_poly.clipped_list);
		}
	else
		calc_path_pos(&ado_path_poly, &path_point, scale, vs.pa_closed);
	}
while (--count >= 0)
	{
	copy_structure(s,d,sizeof(*d) );
	act_size(d, op, scale);
	act_rotate( d, op, scale);
	act_move(d, &op->move, scale);
	if (path)
		{
		d->x += path_point.x;
		d->y += path_point.y;
		d->z += path_point.z;
		}
	s++;
	d++;
	}
return(1);
}

#define TOOBIG 10000

/* Transform 3-d pointlist into 2-d pointlist doing perspective
   calculations the cheap way */
static
calc_zpoly(s, d, count)
register Vertex *s;
register Point *d;
int count;
{
int x, y, z;

while (--count >= 0)
	{
	z = s->z + GROUND_Z;
	if (z < 1)
		return(0);
	x = d->x = sscale_by(s->x-XMAX/2, GROUND_Z, z) + XMAX/2;
	y = d->y = sscale_by(s->y-YMAX/2, GROUND_Z, z) + YMAX/2;
	if (x < -TOOBIG || x > TOOBIG || y < -TOOBIG || y > TOOBIG)
		return(0);
	d += 1;
	s += 1;
	}
return(1);
}

otoobig()
{
continu_line(a3d_100 /* "Shape too big, sorry." */);
}

/* Take a 3-d poly and run it through transformation stack.  Then
   put result through a perspective calculation to yield a 2-D
   result. */
static
ado_transform(points, count, scale, dest)
Vector *points;
int count;
int scale;
Point *dest;
{
struct ado_setting *as;
int path;

as = &vs.move3;
path = got_path;
while (as != NULL)
	{
	move_vpoly(points, points, count, as, scale, path);
	path = 0;	/* Just first guy gets a path */
	scale = SCALE_ONE;
	as = as->next;
	}
if (!calc_zpoly(points, dest, count))
	{
	otoobig();
	return(0);
	}
return(1);
}


/* figure out the center of graphic element */
default_center(v)
Vertex *v;
{
get_ado_cel();
v->x = ado_cel->x + ado_cel->w/2;
v->y = ado_cel->y + ado_cel->h/2;
v->z = 0;
if (is_vector())
	{
	if (make_ado_poly())
		{
		extern int pxmin, pxmax, pymin, pymax;
		find_pminmax(&ado_spline1);
		v->x = (pxmin + pxmax)/2;
		v->y = (pymin + pymax)/2;
		free_ado_poly();
		}
	}
}

/* Given two polys, generate a poly 'tween' the two.  This is
   a convenient place to promote the 2-D poly to a 3-D one too. */
xform_to_ado_poly(sp0, sp1, d, count, scale)
Poly *sp0, *sp1;
Vertex *d;
int count, scale;
{
Vertex da0, da1;
LLpoint *s0, *s1;
int tween_scale;
int count1  = count-!sp0->closed;
int i;

if (count <= 0)
	return;
s0 = sp0->clipped_list;
s1 = sp1->clipped_list;
if (scale != 0 && scale != SCALE_ONE &&
	sp0->pt_count != sp1->pt_count && imax(sp0->pt_count,sp1->pt_count) > 6)
	/* do a tween where we generate intermediate points */
	{
	for (i=0; i<count;  i++)
		{
		tween_scale = uscale_by(SCALE_ONE, i, count1);
		calc_path_pos(sp0, &da0, tween_scale, sp0->closed);
		/* path position is movement offset so add in 1st point position */
		da0.x += s0->x;
		da0.y += s0->y;
		da0.z += s0->z;
		calc_path_pos(sp1, &da1, tween_scale, sp0->closed);
		da1.x += s1->x;
		da1.y += s1->y;
		da1.z += s1->z;
		d->x = da0.x + itmult(da1.x - da0.x, scale);
		d->y = da0.y + itmult(da1.y - da0.y, scale);
		d->z = da0.z + itmult(da1.z - da0.z, scale);
		d += 1;
		}
	}
else
	{
	while (--count>=0)
		{
		d->x = s0->x + itmult(s1->x - s0->x, scale);
		d->y = s0->y + itmult(s1->y - s0->y, scale);
		d->z = 0;
		d += 1;
		s0 = s0->next;
		s1 = s1->next;
		}
	}
}

/* Presuming we've already done a make_ado_poly this routine is
   all you need to get the 2-D polygon in screen coordinates
   to map graphic element into.  This result polygon is ado_dpoly. */
static
ado_calc_poly(form,scale)
Vscreen *form;
int scale;
{
/* If it's a vector source let's go tween it */
if (vs.ado_source == OPS_SPLINE || vs.ado_source == OPS_POLY)
	{
	ado_spline1.closed = vs.closed_curve||vs.fillp;
	xform_to_ado_poly(&ado_spline1,&ado_spline2,ado_vpoly,ado_ptcount,scale);
	}
/* If it's a raster source just make a rectangular polygon */
else
	{
	sq_vpoly(form, ado_vpoly);	/* get initial polygon fitting cel/frame */
	}
return(ado_transform(ado_vpoly, ado_ptcount, scale, ado_dpoly)); 
}


/* This is the 'auto vec' to render optics on one frame */
static
twirl1(ix, frames,scale)
int ix,frames,scale;
{
Vscreen *tf = NULL;
int success = 0;

		/* ~~~ */
switch (vs.ado_source)
	{
	case 0:		/* screen */
		if (vs.draw_mode != I_OPAQUE)
			{
			if (!write_gulp(another_name, uf.p, 64000L) )
				goto BADEND;
			color_form(&uf,vs.inks[0]);
			}
		if ((tf = clone_screen(render_form)) == NULL)
			goto BADEND;
		color_form(render_form,vs.inks[0]);
		break;
	case 1:		/* cel */
		if (!load_temp_cel())
			goto BADEND;
		tf = (Vscreen *)cel;
		if (need_fit_cel(cel))
			cfit_cel(cel, render_form->cmap);
		break;
	}
if (!make_ado_poly())
	goto BADEND;
if (!ado_calc_poly(tf, scale))
	goto BADEND;
switch (vs.ado_source)
	{
	case OPS_SCREEN:
	case OPS_CEL:
		if (!raster_transform(tf, ado_dpoly, 2))
			{
			goto BADEND;
			}
		if (vs.ado_outline)
			{
			int omode;

			omode = vs.draw_mode;
			vs.draw_mode = 0;
			render_outline(ado_dpoly, ado_ptcount);
			vs.draw_mode = omode;
			}
		break;
	case OPS_POLY:
	case OPS_SPLINE:
		if (!rado_poly(ado_dpoly, 
			tween_pt_count(&ado_spline1, &ado_spline2, scale), vs.fillp,
			vs.ado_source == OPS_SPLINE))
			{
			free_ado_poly();
			goto BADEND;
			}
		break;
	}
switch (vs.ado_source)
	{
	case 0:	/* screen source */
		free_screen(tf);
		tf = NULL;
		if (vs.draw_mode != I_OPAQUE)
			{
			if (!read_gulp(another_name, uf.p, 64000L))
				goto BADEND;
			jdelete(another_name);
			}
		break;
	case 1:	/* cel source */
		free_cel(cel);
		cel = NULL;
		break;
	}
free_ado_poly();
success = 1;
BADEND:
switch (vs.ado_source)
	{
	case 0:
		free_screen(tf);
		break;
	case 1:
		free_cel(cel);
		cel = NULL;
		break;
	}
return(success);
}

/* Transform one 0-TWOPI based ado-op angle into something we can
   display on a slider */
iscale_theta()
{
rot_theta.x = rscale_by(vs.move3.spin_theta.x, vs.ado_turn, TWOPI);
rot_theta.y = rscale_by(vs.move3.spin_theta.y, vs.ado_turn, TWOPI);
rot_theta.z = rscale_by(vs.move3.spin_theta.z, vs.ado_turn, TWOPI);
}


/* Transform one turn slider into 0-TWOPI based angle.  Copes with
   sliders being in degrees, 1/8 circles, 1/4 circle, etc. */
static
nscale_theta(s, d, offset)
WORD *s, *d, offset;
{
d[offset] = rscale_by(s[offset], TWOPI, vs.ado_turn);
}


/* feelme for one of the optics x/y/z sliders */
ado_xyz_slider(m)
Flicmenu *m;
{
feel_qslider(m);
if (inspin)
	nscale_theta(&rot_theta, &vs.move3.spin_theta, m->identity);
}

/* zero out an optics x/y/z slider.  Usual response to right click over
   optics x/y/z slider */
xyz_zero_sl(m)
Flicmenu *m;
{
zero_sl(m);
if (inspin)
	nscale_theta(&rot_theta, &vs.move3.spin_theta, m->identity);
}


/* Gnarly math I wrote for Aegis Animator and then tried to forget.
   Make a 'conjugacy' matrix to compensate for axis tilt.  Ie
   we'll go ahead and do the op-rotation as if there were no
   axis tilt, but bracket both sides with another rotation and
   it's inverse.  Given the axis this guy figures out what
   bracketing rotations are necessary. (Boy, it's a good thing
   I know Linear Algebra.) */
static
make_rot_op()
{
find_conjugates(&vs.move3);
}


/* The center's point list and place to put transformed 3d points, and
   place to put 2d points */
static Vertex csvecs[4] = {{0, 0, 0}, {-36, 0, 0}, {0, -36, 0}, {0, 0, 36}};
static Vertex cdvecs[4];
static Point cdpts[4];

/* Display center.  */
static
dcenter(dotout,scale)
Vector dotout;	/* marqi it?  erase it? */
int scale;
{
register Vertex *pt;
int i, theta;
int sizer;

sizer = (vs.ado_mode == ADO_SIZE);
pt = (sizer ? &vs.move3.size_center : &vs.move3.spin_center);
for (i=0; i<4; i++)
	{
	cdvecs[i].x = csvecs[i].x;
	cdvecs[i].y = csvecs[i].y;
	cdvecs[i].z = csvecs[i].z;
	if ((theta = vs.move3.itheta1) != 0)
		partial_rot(theta, &cdvecs[i].x, &cdvecs[i].y);
	if ((theta = vs.move3.itheta2) != 0)
		partial_rot(theta, &cdvecs[i].y, &cdvecs[i].z);
	cdvecs[i].x += pt->x;
	cdvecs[i].y += pt->y;
	cdvecs[i].z += pt->z;
	}
if (ado_transform(cdvecs, 4, scale, cdpts))
	{
	for (i=1; i<4; i++)
		cline(cdpts[0].x, cdpts[0].y, cdpts[i].x, cdpts[i].y, dotout);
	return(TRUE);
	}
else
	return(FALSE);
}


tween_pt_count(Poly *p1, Poly *p2, int scale)
{
if (scale == 0)
	return(p1->pt_count);
else if (scale == SCALE_ONE)
	return(p2->pt_count);
else
	return(imax(p1->pt_count,p2->pt_count));
}

/* See marqi'd wireframe and center */
static
see_ado_poly(scale)
int scale;
{
int pct;

pct = (is_vector() ? tween_pt_count(&ado_spline1, &ado_spline2, scale) : 4);
marqidata.mod = 0;
if (!dcenter(marqidot, scale))
	return(FALSE);
msome_vector(ado_dpoly, 
	pct, marqidot, &marqidata,
	is_vector() && !is_closedp());
return(TRUE);
}


/* Go do a wire-frame simulation of what ado move will look like
   so user can get a sense of what the timing will be before
   he goes to the pixel perfect (and slow) preview or even
   (gasp) to render it. */
static
ado_preview()
{
int occolor;
int i;
long clock;
Point *lado_dpoly;
int scale, lscale;

make_ado_poly();
if ((lado_dpoly = begmem(sizeof(Point)*ado_ptcount)) == NULL)
	{
	free_ado_poly();
	return;
	}
mouse_on = 0;
save_undo();
make_rot_op();
occolor = vs.ccolor;
vs.ccolor = sbright;
find_range();
get_ado_cel();
clock = get80Hz();
/* ~~~ */
for (i=0; i<tr_frames; i++)
	{
	scale = calc_time_scale(i, tr_frames);
	if (!ado_calc_poly(ado_cel,scale))
		break;
	if (i != 0)
		{
		undo_poly(lado_dpoly, ado_ptcount);
		if (!dcenter(copydot, lscale))
			break;
		}
	see_ado_poly(scale);
	copy_structure(ado_dpoly, lado_dpoly, sizeof(*lado_dpoly)*ado_ptcount );
	clock += fhead.speed;
	c_input();
	while (clock > get80hz())
		wait_sync();
	if (clock < get80hz())
		clock = get80hz();
	if (key_hit || RJSTDN)
		break;
	lscale = scale;
	}
free_ado_poly();
freemem(lado_dpoly);
unundo();
vs.ccolor = occolor;
mouse_on = 1;
return;
}

/* So many ways to exit from ado_preview it's easier to do the
   necessary  menu hiding and restoring here in a little bracketing
   routine */
mado_preview()
{
hide_mp();
ado_preview();
draw_mp();
}


/* the response to the 'do it' button */
mauto_ado()
{
int omulti;

hide_mp();
omulti = vs.multi;
vs.multi = 1;
make_rot_op();
if (push_inks())
	{
	if (push_cel())
		{
		/* ~~~ */
		doauto(twirl1);
		pop_cel();
		}
	else
		goto NOTEMP;
	pop_inks();
	}
else
	goto NOTEMP;
vs.multi = omulti;
draw_mp();
return;
NOTEMP:
noroom();
draw_mp();
}

/* Clear top of transformation stack */
static
ado_clear()
{
a3d_disables();
copy_structure(&default_vs.move3, &vs.move3, sizeof(vs.move3) );
rot_theta.x = rot_theta.y = rot_theta.z = 0;
default_center(&vs.move3.spin_center);
copy_structure(&vs.move3.spin_center, &vs.move3.size_center,
	sizeof(&vs.move3.size_center));
free_list((Name_list *)vs.move3.next);
vs.move3.next = NULL;
}

/* Clear all optics motion */
static
ado_clear_p()
{
ado_clear();
jdelete(ppoly_name);
a3d_disables();
}

/* Set up optics for simple spin about horizontal axis */
static
auto_twirl()
{
ado_clear_p();
vs.move3.spin_theta.x = TWOPI;
}

/* Set up optics for simple spin in plane */
static
auto_spin()
{
ado_clear_p();
vs.move3.spin_theta.z = TWOPI;
}

/* set up optics for simple spin about vertical axis */
static
auto_whirl()
{
ado_clear_p();
vs.move3.spin_theta.y = TWOPI;
}

/* Set move to a spin in plane while moving back */
static
auto_spin_small()
{
ado_clear_p();
vs.move3.move.z = 500;
vs.move3.spin_theta.z = TWOPI;
}

/* Move straight back */
static
auto_pull_back()
{
ado_clear_p();
vs.move3.move.z = 500;
}

/* preset motion for stretch horizontally while squishing vertically */
static
auto_squash()
{
ado_clear_p();
vs.move3.xp = 100;
vs.move3.xq = 50;
vs.move3.yp = 50;
vs.move3.yq = 100;
vs.move3.size_center.y = YMAX-1;
}


#define CLK_RAD 24
/* Draw a ray for a clock to help user tell where he is during real-time
   sampled path */
static
clock_line(theta, dotout)
int theta;
Vector dotout;
{
Point clk;

polar(theta-TWOPI/4, CLK_RAD, &clk);
cline(XMAX/2, CLK_RAD, XMAX/2+clk.x, CLK_RAD+clk.y, dotout);
}

extern LLpoint *start_polyt(), *poly_add_point();


/* Gather a sampled path from user mouse move */ 
static
sample_path(delay, maxpts, clock)
int delay, maxpts,clock;
{
register LLpoint *this;
int i, theta;

working_poly.closed = 1;
if ((this = start_polyt()) == NULL)
	return(0);
if (clock)
	clock_line(0, sdot);
for (i=0; i<maxpts; i++)
	{
	if (PDN)
		{
		if ((this = poly_add_point()) == NULL)
			goto OUT;	
		sdot(this->x, this->y);
		}
	else
		break;
	theta = rscale_by(TWOPI, i, maxpts);
	if (clock)
		clock_line(theta, sdot);
	timed_input(delay);
	if (clock)
		clock_line(theta, copydot);
	}
OUT:
unundo();
return(1);
}

/* try to calculate and then draw transformed poly */
static
calc_see_ado_poly(form, scale)
Vscreen *form;	/* source cel if any. */
int scale;
{
if (ado_calc_poly(form,scale))
	{
	see_ado_poly(scale);
	return(1);
	}
else
	return(0);
}


/* Response to pendown over drawing area when path sub-panel is
   being displayed */
static
make_path()
{
extern int tr_frames;
int ok;

if (!push_screen())
	{
	noroom();
	return;
	}
make_ado_poly();
make_rot_op();
get_ado_cel();
ok = calc_see_ado_poly(ado_cel,SCALE_ONE);
free_ado_poly();
if (ok)
	{
	save_undo();
	wait_click();
	if (PJSTDN)
		{
		curveflag = (vs.ado_path == PATH_SPLINE);
		switch (vs.ado_path)
			{
			case PATH_SPLINE:
			case PATH_POLY:
				is_path = 1;
				make_poly();
				is_path = 0;
				break;
			case PATH_SAMPLED:
				sample_path(5, 1024, 0);
				break;
			case PATH_CLOCKED:
				find_range();
				sample_path(fhead.speed, tr_frames, 1);
				break;
			}
		curveflag = 0;
		save_poly(ppoly_name,  &working_poly);
		poly_nopoints(&working_poly);
		a3d_disables();
		}
	}
pop_screen();
save_undo();
}

/* keep sizing sliders in reasonable range */
static
check_prop(pq)
WORD *pq;
{
if (pq[0] > 100)
	{
	pq[1] = rscale_by(pq[1], 100, pq[0]);
	pq[0] = 100;
	}
}

/* Response to click over drawing area.  Lets user spin things around
   stretch, scale, etc. by moving around mouse instead of poking numbers
   into sliders. */
static
mouse_move()
{
int lastx, lasty, dx, dy;
int opong, oease;
Vertex *a3d_vertex;
WORD *xvertex, *yvertex;
struct ado_setting oset;
WORD *xplace, *yplace;
WORD use_y, use_x, mouse_xyz;
WORD pixelfac;
int xp, yp, zp;
int remake_op;
int calc_ok;

remake_op = 0;
if (vs.ado_mode == ADO_PATH)
	{
	make_path();
	return;
	}
get_ado_cel();
use_y = (vs.ado_mouse < 3);
use_x = 1;
mouse_xyz = 0;
pixelfac = 1;
make_ado_poly();
switch (vs.ado_mode)
	{
	case ADO_SPIN:
		mouse_xyz = 1;
		switch (vs.ado_spin)
			{
			case SPIN_CENTER:
				a3d_vertex = &vs.move3.spin_center;
				break;
			case SPIN_AXIS:
				a3d_vertex = &vs.move3.spin_axis;
				remake_op = 1;
				break;
			case SPIN_TURNS:
				a3d_vertex = &vs.move3.spin_theta;
				pixelfac = 10;
				break;
			}
		break;
	case ADO_MOVE:
		a3d_vertex = &vs.move3.move;
		mouse_xyz = 1;
		break;
	case ADO_SIZE:
		pixelfac = 1;
		if (vs.ado_size == 0)	/* center */
			{
			mouse_xyz = 1;
			a3d_vertex = &vs.move3.size_center;
			}
		else
			{
			switch (vs.ado_szmouse)
				{
				case 0:  /* Proportional */
					xvertex = &vs.move3.bp;
					use_x = 1;
					use_y = 0;
					break;
				case 1:	/* XY */
					xvertex = &vs.move3.xp;
					yvertex = &vs.move3.yp;
					use_x = 1;
					use_y = 1;
					break;
				case 2: /* X */
					xvertex = &vs.move3.xp;
					use_x = 1;
					use_y = 0;
					break;
				case 3: /* Y */
					yvertex = &vs.move3.yp;
					use_x = 0;
					use_y = 1;
					break;
				}
			}
		break;
	}
if (mouse_xyz)
	{
	xvertex = yvertex = (WORD *)a3d_vertex;
#define XPOS 0
#define YPOS 1
#define ZPOS 2
	/* Spinning around x axis want cursor to move ywards, so have
	   the following... */
	if (vs.ado_mode == ADO_SPIN && vs.ado_spin == SPIN_TURNS)
		{
		switch (vs.ado_mouse)
			{
			case 0:	/* XY */
				xvertex += YPOS;
				yvertex += XPOS;
				break;
			case  1: /* XZ */
				yvertex += XPOS;
				xvertex += ZPOS;
				break;
			case 2: /* ZY */
				yvertex += ZPOS;
				xvertex += YPOS;
				break;
			case 3: /* Z */
				xvertex += ZPOS;
				break;
			case 4: /* Y */
				xvertex += YPOS;
				break;
			case 5: /* X */
				yvertex += XPOS;
				use_x = 0;
				use_y = 1;
				break;
			}
		}
	else
		{
		switch (vs.ado_mouse)
			{
			case 0:	/* XY */
				xvertex += XPOS;
				yvertex += YPOS;
				break;
			case  1: /* XZ */
				xvertex += XPOS;
				yvertex += ZPOS;
				break;
			case 2: /* ZY */
				xvertex += ZPOS;
				yvertex += YPOS;
				break;
			case 3: /* Z */
				xvertex += ZPOS;
				break;
			case 4: /* Y */
				yvertex += YPOS;
				use_x = 0;
				use_y = 1;
				break;
			case 5: /* X */
				xvertex += XPOS;
				break;
			}
		}
	}
copy_form(render_form, &uf);
make_rot_op();
if (!ado_calc_poly(ado_cel,SCALE_ONE))
	goto OUT;
see_ado_poly(SCALE_ONE);
wait_click();
if (!PJSTDN)
	goto OUT;
/* Save a copy of old way in case they cancel... */
copy_structure(&vs.move3, &oset, sizeof(oset) );
opong = vs.ado_pong;
oease = vs.ado_ease;
vs.ado_pong = vs.ado_ease = 0;

/* I'm fooling the compiler.  Conceivably could break on a different compiler
   on a processor with a wide bus especially.  Structure allignment dependency.
   */
lastx = grid_x;
lasty = grid_y;
for (;;)
	{
	dx = grid_x - lastx;
	dy = grid_y - lasty;
	lastx = grid_x;
	lasty = grid_y;
	if (!dcenter(copydot, SCALE_ONE))
		{
		copy_structure(&oset, &vs.move3, sizeof(oset) ); /* restore old way */
		break;
		}
	undo_poly(ado_dpoly, ado_ptcount);
	if (use_x)
		*xvertex += dx*pixelfac;
	if (use_y)
		*yvertex += dy*pixelfac;
	if (!ado_calc_poly(ado_cel,SCALE_ONE))
		{
		copy_structure(&oset, &vs.move3, sizeof(oset) ); /* restore old way */
		break;
		}
	if (remake_op)
		make_rot_op();
	see_ado_poly(SCALE_ONE);
	wait_input();
	if (key_hit || RJSTDN)
		{
		copy_structure(&oset, &vs.move3, sizeof(oset) ); /* restore old way */
		break;
		}
	if (PJSTDN)
		break;
	}
vs.ado_ease = oease;
vs.ado_pong = opong;
if (vs.ado_mode == ADO_SIZE)
	{
	check_prop(&vs.move3.bp);
	check_prop(&vs.move3.xp);
	check_prop(&vs.move3.yp);
	}
OUT:
unundo();
free_ado_poly();
}

/* Hide menus and then go move things around with the mouse above */
static
mmouser()
{
hide_mp();
mouse_move();
draw_mp();
}

/* What do we do in response to a pull-down selection? */
static
ado_selit(menu, sel)
int menu, sel;
{
char *bufs[4];
char buf1[16], buf2[16];

hide_mp();
switch(menu)
	{
	case 0:
		switch (sel)
			{
			case 0:
				ado_clear_p();
				break;
			case 1:
				auto_pull_back();
				break;
			case 2:
				auto_spin();
				break;
			case 3:
				auto_twirl();
				break;
			case 4:
				auto_whirl();
				break;
			case 5:
				auto_spin_small();
				break;
			case 6:
				auto_squash();
				break;
			case 7:
				go_files(8);
				a3d_disables();
				break;
			}
		break;
	case 1:
		switch (sel)
			{
			case 0:	 /* ease */
				vs.ado_ease = !vs.ado_ease;
				break;
			case 1:	 /* ease out */
				vs.ado_ease_out = !vs.ado_ease_out;
				break;
			case 2:
				vs.ado_tween = !vs.ado_tween;
				break;
			case 3:
				vs.ado_pong = !vs.ado_pong;
				break;
			case 4:
				vs.ado_reverse = !vs.ado_reverse;
				break;
			case 5:
				vs.ado_complete = !vs.ado_complete;
				break;
			}
		break;
	case 2:
		switch (sel)
			{
			case 0:		/* frame */
				vs.ado_source = OPS_SCREEN;
				break;
			case 1:		/* cel */
				vs.ado_source = OPS_CEL;
				break;
			case 2:
				vs.ado_source = OPS_POLY;
				break;
			case 3:
				vs.ado_source = OPS_SPLINE;
				break;
			case 5:		/* outline */
				vs.ado_outline = !vs.ado_outline;
				break;
			}
		break;
	}
draw_mp();
}

/* one shot structure that's effectively the header to a .opt file */
struct magic_moves
	{
	WORD magic;
	WORD moves;
	};

/* Load up transformation stack from some file somebody must have liked
   sometime... */
static
load_a3d(title)
char *title;
{
struct magic_moves mm;
int fd;
int i;
struct ado_setting *as;

if ((fd = jopen(title, 0)) == 0)
	{
	cant_find(title);
	return(0);
	}
ado_clear();
if (jread(fd, &mm, (long)sizeof(mm) ) != sizeof(mm) )
	{
	truncated(title);
	goto BADOUT;
	}
if (mm.magic != A3D_MAGIC)
	{
	continu_line(a3d_101 /* "Not a good OPTICS file" */);
	goto BADOUT;
	}
as = NULL;
i = mm.moves;
while (--i >= 0)
	{
	if (jread(fd, &vs.move3, sizeof(vs.move3) ) != sizeof(vs.move3) )
		{
		truncated(title);
		goto BADOUT;
		}
	vs.move3.next = as;
	if (i != 0)
		{
		do_move_along();
		}
	as = vs.move3.next;
	}
jclose(fd);
return(1);
BADOUT:
	jclose(fd);
	return(0);
}

/* Try and save the transformation stack */
static
well_save_a3d(title)
char *title;
{
struct magic_moves mm;
int fd;
int i;
struct ado_setting *as;

mm.magic = A3D_MAGIC;
i = mm.moves = els_in_list(&vs.move3);
if ((fd = jcreate(title)) == 0)
	{
	cant_create(title);
	return(0);
	}
if (jwrite(fd, &mm, (long)sizeof(mm)) != sizeof(mm) )
	{
	truncated(title);
	goto BADOUT;
	}
while (--i >= 0)
	{
	as = list_el(&vs.move3, i);
	if (jwrite(fd, as, sizeof(vs.move3) ) != sizeof(*as) )
		{
		truncated(title);
		goto BADOUT;
		}
	}
jclose(fd);
return(1);
BADOUT:
	jclose(fd);
	return(0);
}

/* Save current transformation stack */
static
save_a3d(title)
char *title;
{
if (!well_save_a3d(title) )
	{
	jdelete(title);		/* delete partial save */
	return(0);
	}
return(1);
}

/* Put up file requestor and then probably load up optics file */
qload_a3d()
{
char *title;

if ((title =  get_filename(a3d_102 /* "Load Optics Move?" */, 
	".OPT"))!=NULL)
	{
	load_a3d(title);
	}
}

/* Put up file requestor and then probably save optics file. */
qsave_a3d()
{
char *title;

if ((title =  get_filename(a3d_104 /* "Save Optics Move?" */, 
	".OPT"))!=NULL)
	{
	if (overwrite_old(title))
		save_a3d(title);
	}
}


/* Point ado_cel to the right raster source */
static
get_ado_cel()
{
if (vs.ado_source == OPS_CEL)
	ado_cel = cel;
else
	ado_cel = (Vcel *)&vf;
}

/* just let em see the path without changing it */
mview_path()
{
hide_mp();
if (load_poly(ppoly_name))
	{
	save_undo();
	curveflag = (vs.ado_path == 0);
	is_path = 1;
	rub_wpoly();
	is_path = 0;
	curveflag = 0;
	wait_click();
	unundo();
	poly_nopoints(&working_poly);
	}
draw_mp();
}

/* let 'em move around the points on an existing path */
edit_path()
{
Poly opoly;

hide_mp();
if (load_poly(ppoly_name) )
	{
	curveflag = (vs.ado_path == 0);
	is_path = 1;
	move_poly_points();
	is_path = 0;
	curveflag = 0;
	save_poly(ppoly_name, &working_poly);
	poly_nopoints(&working_poly);
	}
draw_mp();
}

/* Duplicate top of transformation stack. */
static
do_move_along()
{
struct ado_setting *as;

make_rot_op();
if ((as = begmem(sizeof(*as)))==NULL)
	return(0);
copy_structure(&vs.move3,as,sizeof(*as));
vs.move3.next = as;
return(1);
}

/* response to 'continue move' */
move_along(m)
Flicmenu *m;
{
struct ado_setting *as;
LLpoint *first, *last;

hilight(m);
if (do_move_along())
	{
	if (got_path)	/* fold path into move op */
		{
		if (!vs.pa_closed)	
			{
			if (load_poly(ppoly_name))
				{
				as = vs.move3.next;
				first = working_poly.clipped_list;
				last = list_el(first, working_poly.pt_count-1);
				as->move.x += last->x - first->x;
				as->move.y += last->y - first->y;
				as->move.z += last->z - first->z;
				poly_nopoints(&working_poly);
				}
			}
		}
	wait_a_jiffy(15);
	}
draw_sel(m);
}

/* Lets go to the optics editor folks! */
go_ado()
{
Flicmenu *omenu;
Pull *orpc;
Pull *ocp;

ado_clear();
if (jexists(optics_name))
	load_a3d(optics_name);
arrange_a3d_menu();
a3d_disables();
unzoom();
clip_tseg();
omenu = cur_menu;	/* save old 'panel' (quick) menu */
cur_menu = &a3d_menu;	/* and current panel menu */
ocp = cur_pull;
cur_pull = &root_pull;
orpc = root_pull.children;
root_pull.children = &presets_pull;	/* go to ADO pulldowns */
a3d_pull_disables();
draw_mp();
sub_menu_loop(ado_selit,mmouser);
hide_mp();
cur_menu = omenu;
root_pull.children = orpc;	/* and back to regular ones */
cur_pull = ocp;
save_a3d(optics_name);
rezoom();
}

