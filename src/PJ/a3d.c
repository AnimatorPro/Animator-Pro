/*	a3d.c - This file contains most of the code for the optics section. */

#include <string.h>
#include "errcodes.h"
#include "ptrmacro.h"
#include "jimk.h"
#include "auto.h"
#include "a3d.h"
#include "a3dpul.h"
#include "commonst.h"
#include "flx.h"
#include "inks.h"
#include "memory.h"
#include "poly.h"
#include "rastrans.h"
#include "softmenu.h"
#include "tween.h"

static Errcode ado_mouse_ptfunc(Pentool *pt, Wndo *w);
static Errcode eload_a3d(char *name);
static Errcode save_a3d(char *title);
static Errcode do_move_along(void);

/************** Stuff for 2-Dimensional point-lists *****************/
typedef struct poly2
	{
	int count;
	Short_xy *p2;
	} Poly2;

static Errcode poly2_init(Poly2 *p, int count)
/* Initialize Poly2 structure and allocate buffer for count points */
{
clear_struct(p);
if ((p->p2 = pj_malloc(count*sizeof(Short_xy))) == NULL)
	return(Err_no_memory);
p->count = count;
return(Success);
}

static void poly2_cleanup(Poly2 *p)
/* Cleanup Poly2 structure and free point buffer */
{
pj_gentle_free(p->p2);
clear_struct(p);
}



/************** Stuff for 3-Dimensional point-lists *****************/
typedef struct poly3
	{
	int count;
	Short_xyz *p3;		/* 3-D point list */
	Short_xyz *p3alloc;	/* allocated 3-D points */
	} Poly3;
static void poly3_cleanup(Poly3 *p);

static Errcode poly3_init(Poly3 *p, int count, Short_xyz *points)
/* Initialize a poly3 structure.  If points is NULL then allocate
 * a array big enough for count 3-D points.
 * In any case allocate a 2-D point array of count elements */
{
clear_struct(p);
if (points == NULL)
	{
	if ((p->p3alloc = p->p3 = pj_malloc(sizeof(*points)*count)) == NULL)
		{
		poly3_cleanup(p);
		return(Err_no_memory);
		}
	}
else
	{
	p->p3 = points;
	}
p->count = count;
return(Success);
}

static void poly3_cleanup(Poly3 *p)
/* Free point arrays of a Poly3 */
{
pj_gentle_free(p->p3alloc);
clear_struct(p);
}

/**************** Optics motion stuff *******************/
static void act_rotate( register Short_xyz *point,
	      			    register struct ado_setting *op,
				        SHORT scale)
/* Do the yaw, pitch, and roll rotation to a 3-d point relative to
   wherever they've twisted the axis.  */
{
register SHORT theta;

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


static void act_size( register Short_xyz *point,
				      register struct ado_setting *op,
				      SHORT scale)
/* Do the x, y, and both scaling to a single 3-d point */
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

static void act_move(Short_xyz *p, Short_xyz *op, SHORT scale)
/* Do the move (translation) part of optics transform to 1 point */
{
int i;

	i = 3;
	while(--i >= 0)
	{
		*(SHORT *)p += itmult(*(SHORT *)op, scale);
		p = OPTR(p,sizeof(p->x));
		op = OPTR(op,sizeof(op->x));
	}
}


/**************** Gnarly optics stuff that depends on globals *************/
extern char curveflag;	/* draw points as polygon or curve? */
extern int is_path;		/* Use path or curve tension cont. bias? */

/* Variables to hold our graphic element (source for optics) */
static Rcel *ado_cel;					/* if it's a raster element */
static Poly ado_poly_el;
static Tween_state ado_tween_el;
static Tw_tlist ado_tlist;
static Poly ado_path_poly;	/* a place for the path in RAM */

static Boolean is_poly_el(void)
{
	return(vs.ado_source == OPS_SPLINE || vs.ado_source == OPS_POLY);
}

/* is it a vector element or a raster element? */
static Boolean is_vector(void)
{
	return(vs.ado_source == OPS_TWEEN || is_poly_el());
}

static void get_ado_cel(void)
/* Point ado_cel to the right raster source */
{
	if (vs.ado_source == OPS_THECEL)
		ado_cel = thecel->rc;
	else
		ado_cel = vb.pencel;
}

static void ado_unload_element(void);

static Errcode ado_load_element(void)
/* Load ado element from disk  */
{
Errcode err = Success;

if (got_path)
	{
		if ((err = load_a_poly(ppoly_name, &ado_path_poly)) < Success)
			goto ERR;
		if(ado_path_poly.pt_count == 0)
		{
			/* path polys without any points screw things up */
			pj_delete(ppoly_name);
			a3d_disables();
			err = Err_abort;
			goto ERR;
		}
	}
switch (vs.ado_source)
	{
	case OPS_POLY:
	case OPS_SPLINE:
		if ((err = load_a_poly(poly_name,&ado_poly_el)) < Success)
			goto ERR;
		break;
	case OPS_TWEEN:
		init_tween_state(&ado_tween_el);
		if ((err = load_tween(tween_name, &ado_tween_el)) < Success)
			goto ERR;
		if ((err =  ts_to_tw_list( &ado_tween_el,  
			(vs.closed_curve || vs.fillp), &ado_tlist)) 
			< Success)
			goto ERR;
		break;
	default:
		break;
	}
return(Success);
ERR:
ado_unload_element();
return(err);
}

static void ado_unload_element(void)
/* Free up loaded ado element */
{
switch (vs.ado_source)
	{
	case OPS_POLY:
	case OPS_SPLINE:
		free_polypoints(&ado_poly_el);
		break;
	case OPS_TWEEN:
		trash_tw_list(&ado_tlist);
		trash_tween_state(&ado_tween_el);
		break;
	default:
		break;
	}
free_polypoints(&ado_path_poly);	/* free up path */
}



static int 
move_vpoly( 
			Short_xyz *s, /* Point list before transformation */
			Short_xyz *d, /* It's ok for s & d to point to same list */
            int count,		/* point count */
			struct ado_setting *op,		/* the transformation */
			int scale,	/* How far into this tranformation?  0 to SCALE_ONE */
			Poly *path_poly,
			int path_type,
			Boolean path_closed
			)	/* We got a path to cope with too? */
/* Do one full optics transformation to a list of points */
{
Short_xyz path_point;
Poly sp_poly;
Errcode err;

	if (path_poly != NULL)
	{
		if (path_type == 0)	/* splining... */
		{
			is_path = 1;	/* tell spline to use path tension cont bias */
			err = make_sp_poly(path_poly, &sp_poly, vs.pa_closed, 16);
			is_path = 0;
			if(err < 0)
				return(err);
			err = calc_path_pos(&sp_poly, &path_point, scale, path_closed);
			pj_free(sp_poly.clipped_list);
			if (err < 0)
				return(err);
		}
		else
		{
			if ((err = calc_path_pos(path_poly, 
				&path_point, scale, path_closed)) < 0)
				return(err);
		}
	}
	while (--count >= 0)
	{
		pj_copy_structure(s,d,sizeof(*d) );
		act_size(d, op, scale);
		act_rotate( d, op, scale);
		act_move(d, &op->move, (SHORT)scale);
		if (path_poly != NULL)
		{
			d->x += path_point.x;
			d->y += path_point.y;
			d->z += path_point.z;
		}
		s++;
		d++;
	}
	return(0);
}

static Errcode ado_transform( Short_xyz *points,
			   int count,
			   int scale,
			   Short_xy *dest)
/* Take a 3-d poly and run it through transformation stack.  Then
   put result through a perspective calculation to yield a 2-D
   result. */
{
Errcode err;
struct ado_setting *as;
Poly *path = NULL;

	as = &vs.move3;
	if (got_path)
		path = &ado_path_poly;
	while (as != NULL)
	{
		if ((err = move_vpoly(points, points, count, 
			as, scale, path, vs.ado_path, vs.pa_closed))
			< Success)
			return(err);
		path = NULL;	/* Just first guy gets a path */
		scale = SCALE_ONE;
		as = as->next;
	}
	err = calc_zpoly(points, dest, count,
		vb.pencel->width>>1, vb.pencel->height>>1,
		512*vb.pencel->width/320);
	return(err);
}

void default_center(Short_xyz *v)
/* figure out the center of graphic element */
{
	if (is_vector())
	{
		if(ado_load_element() >= Success)
		{
		switch(vs.ado_source)
			{
			case OPS_SPLINE:
			case OPS_POLY:
				poly_ave_3d(&ado_poly_el, v);
				break;
			case OPS_TWEEN:
				poly_ave_3d(&ado_tween_el.p0, v);
				break;
			}
		ado_unload_element();
		}
	}
	else
	{
		get_ado_cel();
		v->x = ado_cel->width/2;
		v->y = ado_cel->height/2;
		v->z = 0;
		if(vs.ado_source != OPS_SCREEN)
		{
			v->x += ado_cel->x;
			v->y += ado_cel->y;
		}
	}
}

static void a3d_default_centers(void)
{
default_center(&vs.move3.spin_center);
pj_copy_structure(&vs.move3.spin_center, &vs.move3.size_center,
	sizeof(&vs.move3.size_center));
}

static Errcode poly3_from_poly(Poly3 *p3, Poly *poly)
{
Errcode err;

if ((err = poly3_init(p3, poly->pt_count, NULL)) 
	>= Success)
	poly_to_3d(poly,p3->p3);
return(err);
}

static Errcode ado_calc_poly(Poly2 *dpoly, Rcel *form, SHORT scale)
/* Return a 2-D polygon in dpoly that's the outline of  the
 * optics elements after all transformations and perspective 
 * calculations.   Generally you'll need to do a poly2_free(dpoly)
 * eventually as dpoly's pointlist is allocated here. */
{
Rectangle rect;
int i;
Poly3 vpoly;
Errcode err;
Boolean tclosed;
int tcount;
Short_xyz *tpoints;

	clear_struct(dpoly);
	switch(vs.ado_source)
	{
		case OPS_SPLINE:
		case OPS_POLY: 
			if ((err = poly3_from_poly(&vpoly, &ado_poly_el)) < Success)
				goto OUT;
			break;
		case OPS_TWEEN: 
			tclosed = vs.closed_curve||vs.fillp;
			calc_tween_points(&ado_tlist,tclosed,scale,&tpoints,&tcount);
			poly3_init(&vpoly, tcount+!tclosed, tpoints);
			break;
		case OPS_THECEL: /* if thecel apply turn and stretch for cel poly */
			if ((err = poly3_init(&vpoly, 4, NULL)) < Success)
				goto OUT;
			for(i = 0;i < 4;++i)
				{
				*(Short_xy *)(&vpoly.p3[i]) = thecel->xf.bpoly[i];
				vpoly.p3[i].z = 0;
				}
			break;
		case OPS_SCREEN: 
			/* Screen is vb.pencel, which is a window on the screen
			 * it is always centered on itself so we make x and y 0 */
			if ((err = poly3_init(&vpoly, 4, NULL)) < Success)
				goto OUT;
			rect.x = rect.y = 0;
			rect.width = form->width;
			rect.height = form->height;
			rect_to_xyz(&rect, vpoly.p3);	
			break;
	}
	if ((err = poly2_init(dpoly, vpoly.count)) < Success)
		goto OUT;
	err = ado_transform(vpoly.p3, vpoly.count, scale, dpoly->p2); 
OUT:
	poly3_cleanup(&vpoly);
	return(err);
}

static Errcode rado_poly(Short_xy *ado_s, int ptcount, int curved)
/* Render a 2-Dimensional array of points */
{
Poly p;
LLpoint *list;
Errcode err;
int i;
	if((list = p.clipped_list = pj_malloc(ptcount * sizeof(LLpoint))) == NULL)
		return(Err_no_memory);

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
	err = render_poly(&p, vs.fillp, vs.closed_curve);
	pj_free(p.clipped_list);
	curveflag = 0;
	return(err);
}

static Errcode twirl1(Celcfit *cfit, int ix, int frames, int scale)
/* This is the 'auto vec' to render optics on one frame */
{
Rcel *tf = NULL;
Errcode err;
Xformspec xf;
Tcolxldat tcxl;
Rcel_save oundo;
Poly2 dpoly;
(void)ix;
(void)frames;

	clear_struct(&dpoly);
	clear_struct(&oundo);
	switch(vs.ado_source)
	{
		case OPS_SCREEN:		/* screen */
			if (vs.ink_id != opq_INKID)
			{
				if ((err = report_temp_save_rcel(&oundo, undof)) < Success)
					goto error;
				pj_set_rast(undof, vs.inks[0]);
			}
			if ((tf = clone_pencel(vb.pencel)) == NULL)
				goto nomem_error;
			pj_set_rast(vb.pencel, vs.inks[0]);
			break;
		case OPS_THECEL:		/* cel */
			tf = thecel->rc;
			break;
	}
	if ((err = ado_load_element()) < 0)
		goto error;
	if ((err = ado_calc_poly(&dpoly, tf, scale)) < 0)
		goto error;

	switch (vs.ado_source)
	{
		case OPS_THECEL:
			tcxl.tcolor = thecel->cd.tcolor;
			goto render_rast;
		case OPS_SCREEN:
			tcxl.tcolor = vs.inks[0];
		render_rast:
			if(make_render_cfit(tf->cmap,cfit,tcxl.tcolor))
				tcxl.xlat = cfit->ctable;
			else
				tcxl.xlat = NULL;
			init_xformspec(&xf);
			if(vs.render_under)
				tcxl.tcolor = vs.inks[0];
			copy_mem(dpoly.p2,xf.bpoly,sizeof(xf.bpoly));
			load_poly_minmax(&xf);
			if((err = render_transform(tf,&xf,&tcxl)) < 0)
				goto error;

			if (vs.ado_outline)
			{
			Ink *oink;

				oink = vl.ink;
				id_set_curink(opq_INKID);
				make_render_cashes();
				render_outline(dpoly.p2, dpoly.count);
				free_render_cashes();
				set_curink(oink);
			}
			break;
		case OPS_POLY:
		case OPS_SPLINE:
			if ((err = rado_poly(dpoly.p2, dpoly.count, 
				vs.ado_source == OPS_SPLINE)) < Success)
				goto error;
			break;
		case OPS_TWEEN:
			if ((err = rado_poly(dpoly.p2, dpoly.count,
				vs.tween_spline)) < Success)
				goto error;
			break;
	}
	switch (vs.ado_source)
	{
		case OPS_SCREEN:	/* screen source */
			if (vs.ink_id != opq_INKID)
			{
				if(undof->type != RT_BYTEMAP)
				{
					err = Err_bad_input;
					goto error;
				}
			}
			break;
		case OPS_THECEL:	/* cel source */
			break;
	}
	err = 0;
	goto cleanup;

nomem_error:
	err = Err_no_memory;
	goto error;
error:
cleanup:
	poly2_cleanup(&dpoly);
	ado_unload_element();
	switch (vs.ado_source)
	{
		case OPS_SCREEN:
			pj_rcel_free(tf);
			break;
	}
	report_temp_restore_rcel(&oundo, undof);
	return(err);
}

void iscale_theta(void)
/* Transform one 0-TWOPI based ado-op angle into something we can
   display on a slider */
{
	rot_theta.x = rscale_by(vs.move3.spin_theta.x, vs.ado_turn, TWOPI);
	rot_theta.y = rscale_by(vs.move3.spin_theta.y, vs.ado_turn, TWOPI);
	rot_theta.z = rscale_by(vs.move3.spin_theta.z, vs.ado_turn, TWOPI);
}

static int zscale_by(int x,int p,int q)
/* return x*p/q rounded.  Return BIG_SHORT if q == 0 */
{
if (q == 0)
	return(BIG_SHORT);
else
	return(rscale_by(x,p,q));
}

static void nscale_theta(Short_xyz *s, Short_xyz *d, int ix)
/* Transform one turn slider into 0-TWOPI based angle.  Copes with
   sliders being in degrees, 1/8 circles, 1/4 circle, etc. */
{
	((SHORT*)d)[ix] = zscale_by(((SHORT*)s)[ix], TWOPI, vs.ado_turn);
}


void ado_xyz_slider(Button *b)
/* feelme for one of the optics x/y/z sliders */
{
	feel_qslider(b);
	if (inspin)
		nscale_theta(&rot_theta, &vs.move3.spin_theta, b->identity);
}

void xyz_zero_sl(Button *m)
/* zero out an optics x/y/z slider.  Usual response to right click over
   optics x/y/z slider */
{
	zero_sl(m);
	if (inspin)
		nscale_theta(&rot_theta, &vs.move3.spin_theta, m->identity);
}

static void make_rot_op(void)
/* Gnarly math I wrote for Aegis Animator and then tried to forget.
   Make a 'conjugacy' matrix to compensate for axis tilt.  Ie
   we'll go ahead and do the op-rotation as if there were no
   axis tilt, but bracket both sides with another rotation and
   it's inverse.  Given the axis this guy figures out what
   bracketing rotations are necessary. (Boy, it's a good thing
   I know Linear Algebra.) */
{
	find_conjugates(&vs.move3);
}


/* The center's point list and place to put transformed 3d points, and
   place to put 2d points */

static Short_xyz csvecs[4] = {{0, 0, 0}, {-36, 0, 0}, {0, -36, 0}, {0, 0, 36}};
static Short_xyz cdvecs[4];
static Short_xy cdpts[4];

static Errcode dcenter(dotout_func dotout, void *dotdat, int scale)
/* Display center.  */
{
register Short_xyz *pt;
int i, theta;
int sizer;
Errcode err;

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
	if ((err = ado_transform(cdvecs, 4, scale, cdpts)) >= 0)
	{
		for (i=1; i<4; i++)
			pj_cline(cdpts[0].x, cdpts[0].y, cdpts[i].x, cdpts[i].y, 
				  dotout, dotdat);
	}
	return(err);
}


static void marqi_ado_poly(Marqihdr *mh,Poly2 *dpoly, int scale)
/* Draw marqi'd wireframe and center */
{
	mh->dmod = mh->smod;
	dcenter(mh->pdot,mh,scale);
	msome_vector(dpoly->p2, dpoly->count,mh->pdot,mh,
				 is_vector() && !vs.closed_curve, sizeof(dpoly->p2[0]));
}

static void undo_ado_poly(Marqihdr *mh,Poly2 *apoly,int scale)
/* Undraw marqi'd wireframe and center */
{
	msome_vector(apoly->p2, apoly->count, undo_marqidot,mh,
				 is_vector() && !vs.closed_curve, sizeof(apoly->p2[0]));
	dcenter(undo_marqidot, mh, scale);
}


static Errcode ado_preview(Boolean doloop)
/* Go do a wire-frame simulation of what ado move will look like
   so user can get a sense of what the timing will be before
   he goes to the pixel perfect (and slow) preview or even
   (gasp) to render it. */
{
Errcode err;
int occolor;
int i;
long clock;
int scale; 
int lscale = 0; /* init to get rid of warning */
Marqihdr mh;
Boolean first = TRUE;
Poly2 dpoly, ldpoly;

	clear_struct(&dpoly);
	clear_struct(&ldpoly);
	hide_mouse();
	save_undo();
	occolor = vs.ccolor;
	if ((err = ado_load_element()) <  Success)
		goto OUT;

	vinit_marqihdr(&mh,1,1);

	make_rot_op();
	vs.ccolor = sbright;
	find_range();
	get_ado_cel();
	clock = pj_clock_1000();

	do
	{
		for (i=0; i<tr_frames; i++)
		{
										/* get fixed point frame pos */
			scale = calc_time_scale(i, tr_frames); 
										/* calc new 2D poly */
			if ((err = ado_calc_poly(&dpoly,ado_cel,scale)) < 0)  
				goto OUT;
			if (!first)					/* erase old wireframe image */
				undo_ado_poly(&mh,&ldpoly,lscale);
			else						/* except for first time through */
				first = !first;
										/* display wireframe */
			marqi_ado_poly(&mh, &dpoly, scale);
			poly2_cleanup(&ldpoly);		/* Free up old 2D poly */
			ldpoly = dpoly;				/* and set old to be current one */
			clear_struct(&dpoly);		/* and mark current one as gone. */
			clock += flix.hdr.speed;	/* Figure out when to advance frame */
			err = poll_abort();			/* Check for user impatience */
			while (clock > pj_clock_1000()) /* Wait until it's time to advance. */
				wait_sync();
			if (clock < pj_clock_1000())	/* Let clock slip if we're slow. */
				clock = pj_clock_1000();
			if(err < Success) 			/* Get out of here on error or abort. */
				goto OUT;
			lscale = scale;				/* Save scale to erase next frame. */
		}
	} while (doloop);
OUT:
	ado_unload_element();
	poly2_cleanup(&dpoly);
	poly2_cleanup(&ldpoly);
	zoom_unundo();
	vs.ccolor = occolor;
	show_mouse();
	return(softerr(err,"opt_view"));
}

void mado_loop(void)
{
	hide_mp();
	ado_preview(TRUE);
	show_mp();
}

void mado_view(void)
/* So many ways to exit from ado_preview it's easier to do the
   necessary  menu hiding and restoring here in a little bracketing
   routine */
{
	hide_mp();
	ado_preview(FALSE);
	show_mp();
}

static int a3d_get_auto_flags(void)
{
int autoflags;

autoflags = (AUTO_UNZOOM|AUTO_HIDEMP|AUTO_PUSHINKS|AUTO_PUSHCEL);
if(vs.ado_source == OPS_THECEL || vs.ink_id == celt_INKID)
	autoflags |= AUTO_USESCEL;
return(autoflags);
}

void mauto_ado(void)
/* the response to the 'do it' button */
{
Celcfit cfit;
int omulti;

	omulti = vs.multi;
	vs.multi = 1;
	make_rot_op();

	init_celcfit(&cfit);
	go_autodraw(twirl1,&cfit,a3d_get_auto_flags());

	vs.multi = omulti;
}

static void ado_clear_top(void)
/* sets top of transform stack to default values */
{
struct ado_setting *next;

	next = vs.move3.next;
	pj_copy_structure(&default_vs.move3, &vs.move3, sizeof(vs.move3) );
	vs.move3.next = next;
	rot_theta.x = rot_theta.y = rot_theta.z = 0;
	a3d_default_centers();
}

void ado_clear_pos(void)
/* Clears top of optics stack and path */
{
	ado_clear_top();
	pj_delete(ppoly_name);
}

static void ado_free_trans(void)
/* free optics transform stack */
{
	free_slist(vs.move3.next);
	vs.move3.next = NULL;
}

static void ado_clear_stack(void)
{
	ado_free_trans();
	ado_clear_top();
}

static void ado_clear(void)
/* Clear everything except for path... */
{
	ado_clear_stack();
	a3d_disables();
}

static void ado_clear_all(void)
/* Clear all optics motion */
{
	ado_clear_stack();
	pj_delete(ppoly_name);
	a3d_disables();
}

static void auto_twirl(void)
/* Set up optics for simple spin about horizontal axis */
{
	ado_clear_all();
	vs.move3.spin_theta.x = TWOPI;
}

static void auto_spin(void)
/* Set up optics for simple spin in plane */
{
	ado_clear_all();
	vs.move3.spin_theta.z = TWOPI;
}

static void auto_whirl(void)
/* set up optics for simple spin about vertical axis */
{
	ado_clear_all();
	vs.move3.spin_theta.y = TWOPI;
}

static void auto_spin_small(void)
/* Set move to a spin in plane while moving back */
{
	ado_clear_all();
	vs.move3.move.z = 500;
	vs.move3.spin_theta.z = TWOPI;
}

static void auto_pull_back(void)
/* Move straight back */
{
	ado_clear_all();
	vs.move3.move.z = 500;
}

static void auto_squash(void)
/* preset motion for stretch horizontally while squishing vertically */
{
	ado_clear_all();
	vs.move3.xp = 100;
	vs.move3.xq = 50;
	vs.move3.yp = 50;
	vs.move3.yq = 100;
	vs.move3.size_center.y = vb.pencel->height - 1;
}

static void clock_line(int theta, dotout_func dotout)
/* Draw a ray for a clock to help user tell where he is during real-time
   sampled path */
{
Short_xy clk;

#define CLK_RAD 24

	polar(theta-TWOPI/4, CLK_RAD, (short *)&clk);
	pj_cline(vb.pencel->width/2, CLK_RAD, vb.pencel->width/2+clk.x, 
		  CLK_RAD+clk.y, dotout);

#undef CLK_RAD 
}

extern LLpoint *start_polyt(), *poly_add_point();


static int sample_path(Poly *poly, int delay, int maxpts, int clock)
/* Gather a sampled path from user mouse move */ 
{
register LLpoint *this;
int i, theta;

	if ((this = start_polyt(poly)) == NULL)
		return(Err_nogood);
	if (clock)
		clock_line(0, ccolor_dot);
	for (i=0; i<maxpts; i++)
	{
 		if (ISDOWN(MBPEN))
		{
			if ((this = new_poly_point(poly)) == NULL)
				goto OUT;	
			ccolor_dot(this->x, this->y, NULL);
		}
		else
			break;

		theta = rscale_by(TWOPI, i, maxpts);
		if (clock)
			clock_line(theta, ccolor_dot);
		mac_vsync_wait_input(KEYHIT,MMOVE|KEYHIT,delay);
		if (clock)
			clock_line(theta, undo_dot);
	}
OUT:
	zoom_unundo();
	return(0);
}

static int calc_see_ado_poly(Rcel *cel, SHORT scale)
/* try to calculate and then draw transformed poly */
{
Errcode err;
Marqihdr mh;
Poly2 dpoly;

	if((err = ado_calc_poly(&dpoly,cel,scale)) >= 0)
	{
		vinit_marqihdr(&mh,1,1);
		marqi_ado_poly(&mh,&dpoly,scale);
	}
	poly2_cleanup(&dpoly);
	return(err);
}


static Errcode make_path(void)
/* Response to pendown over drawing area when path sub-panel is
   being displayed */
{
extern SHORT tr_frames;
Errcode err = Success;
Rcel_save opic;
Poly wpoly;

	clear_struct(&wpoly);
	if ((err = report_temp_save_rcel(&opic, vb.pencel)) < Success)
		return(err);
	make_rot_op();
	get_ado_cel();
	err = calc_see_ado_poly(ado_cel,SCALE_ONE);
	if(err >= 0)
		{
		save_undo();
		wait_click();
 		if (JSTHIT(MBPEN))
			{
			curveflag = (vs.ado_path == PATH_SPLINE);
			switch (vs.ado_path)
				{
				case PATH_SPLINE:
				case PATH_POLY:
					is_path = 1;
					make_poly(&wpoly, vs.pa_closed);
					is_path = 0;
					break;
				case PATH_SAMPLED:
					sample_path(&wpoly,5, 1024, 0);
					break;
				case PATH_CLOCKED:
					find_range();
					sample_path(&wpoly,millisec_to_jiffies(flix.hdr.speed),
						tr_frames, 1);
					break;
				}
			curveflag = 0;
			if(wpoly.pt_count > 0) /* No 0 point paths! */
				save_poly(ppoly_name,  &wpoly);
			free_polypoints(&wpoly);
			a3d_disables();
			}
		}
	report_temp_restore_rcel(&opic, vb.pencel);
	save_undo();
	return(softerr(err, NULL));
}

#ifdef SLUFFED
static int short_clip(short min, short max, short val)
/* make val between min and max */
{
if (val < min)
	val = min;
if (val > min)
	val = min;
return(val);
}

static Errcode short_min_max_clip(short *val, short min, short max)
/* Make sure *val is between min and max. */
{
short v = *val;

if (v < min)
	{
	*val = min;
	return(Err_range);
	}
else if (v > max)
	{
	*val = max;
	return(Err_range);
	}
return(Success);
}

static Errcode box_clip_xyz(Short_xyz *p, int max)
/* Clip an x,y,z point to be in box from (-max,-max,-max) to (max,max,max) */
{
Errcode err;

err = short_min_max_clip(&p->x, -max, max);
err |= short_min_max_clip(&p->y, -max, max);
err |= short_min_max_clip(&p->z, -max, max);
return(err);
}

Errcode clip_ado_setting(struct ado_setting *as)
/* Make sure that optics values are within ranges where arithmetic will
 * work with no overflow. */
{
Errcode err;

err = box_clip_xyz(&as->move, 10000);
err |= box_clip_xyz(&as->spin_center, 10000);
err |= box_clip_xyz(&as->spin_axis, 1000);
err |= box_clip_xyz(&as->size_center, 10000);
err |= box_clip_xyz(&as->spin_theta, TWOPI*10);
err |= short_min_max_clip(&as->xp, -100, 100);
err |= short_min_max_clip(&as->yp, -100, 100);
err |= short_min_max_clip(&as->bp, -100, 100);
err |= short_min_max_clip(&as->xq, 1, 100);
err |= short_min_max_clip(&as->yq, 1, 100);
err |= short_min_max_clip(&as->bq, 1, 100);
return(err);
}
#endif /* SLUFFED */


static void check_prop(SHORT *pq)
/* keep sizing sliders in reasonable range */
{
if (pq[0] > 100)
	{
	if ((pq[1] = zscale_by(pq[1], 100, pq[0])) < 1)
		pq[1] = 1;
	pq[0] = 100;
	}
else if (pq[0] < -100)
	{
	if ((pq[1] = zscale_by(pq[1], 100, -pq[0])) < 1)
		pq[1] = 1;
	pq[0] = -100;
	}
}



static Errcode mouse_move_element(void)
/* Response to click over drawing area.  Lets user spin things around
 * stretch, scale, etc. by moving around mouse instead of poking numbers
 * into sliders. */
{
SHORT *xvertex, *yvertex;
SHORT use_y, use_x, mouse_xyz;
SHORT pixelfac;

Errcode err = Success;
struct ado_setting oset;
int lastx, lasty, dx, dy;
Short_xyz *a3d_vertex;
int remake_op;
Marqihdr mh;
Poly2 dpoly;

/***** The first part of this rather longish routine sets up which
 ***** number are updated by x mouse movements, and which
 ***** number by the y mouse movements.  These will be somewhere
 ***** in the main ado_setting, but have to go through a lot of
 ***** logic to find out exactly where.  However this part of the
 ***** code can't fail, so you can be sure you'll get to the
 ***** next comment like this */
remake_op = 0;
use_y = (vs.ado_mouse < 3);
use_x = 1;
mouse_xyz = 0;
pixelfac = 1;
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
	xvertex = yvertex = (SHORT *)a3d_vertex;

#define XPOS 0
#define YPOS 1
#define ZPOS 2
/* These XPOS defines are not strictly kosher.  If compiler doesn't
 * put shorts right after each other in a structure  this code could break. 
 */

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

#undef XPOS /* done with these */
#undef YPOS
#undef ZPOS

/***** Now we'll do saving of graphics state and initializations so
 ***** we can display the optics element in the current position without
 ***** permenantly placing it on their picture.  Then we wait for them
 ***** to make a  click */

pj_rcel_copy(vb.pencel, undof);
make_rot_op();
get_ado_cel();
if ((err = ado_calc_poly(&dpoly, ado_cel,SCALE_ONE)) < 0)
	goto OUT;
vinit_marqihdr(&mh,1,1);
marqi_ado_poly(&mh,&dpoly, SCALE_ONE);
wait_click();
if (!JSTHIT(MBPEN))		/* if they didn't left click cut out early */
	{
	err = Err_abort;
	goto OUT;
	}


/***** Now set up and go into a loop where as they move the mouse
 ***** the optics element moves accordingly.   Keep  going until
 ***** they make a click.  Right click will forget any changes they've
 ***** made, left click will keep them. */
					/* Save a copy of old settings in case they cancel... */
pj_copy_structure(&vs.move3, &oset, sizeof(oset) );
lastx = icb.mx;
lasty = icb.my;
for (;;)
	{
	dx = icb.mx - lastx;
	dy = icb.my - lasty;
	lastx = icb.mx;
	lasty = icb.my;
	undo_ado_poly(&mh,&dpoly,SCALE_ONE);
	poly2_cleanup(&dpoly);
	if (use_x)
		*xvertex += dx*pixelfac;
	if (use_y)
		*yvertex += dy*pixelfac;
	if ((err = ado_calc_poly(&dpoly, ado_cel,SCALE_ONE)) < 0)
		{
		pj_copy_structure(&oset, &vs.move3, sizeof(oset) ); 
			/* restore old way */
		break;
		}
	if (remake_op)
		make_rot_op();
	marqi_ado_poly(&mh,&dpoly,SCALE_ONE);
	wait_any_input();
	if(JSTHIT(KEYHIT|MBRIGHT))
		{
		pj_copy_structure(&oset, &vs.move3, sizeof(oset) ); 
			/* restore old way */
		break;
		}
	if (JSTHIT(MBPEN))
		break;
	}
if (vs.ado_mode == ADO_SIZE)
	{
	check_prop(&vs.move3.bp);
	check_prop(&vs.move3.xp);
	check_prop(&vs.move3.yp);
	}
OUT:
poly2_cleanup(&dpoly);
zoom_unundo();
return(err);
}

static void ado_mouse_move(void)
{
Errcode err;

if ((err = ado_load_element()) >= Success)
	{
	if (vs.ado_mode == ADO_PATH)
		err = make_path();
	else
		err = mouse_move_element();
	ado_unload_element();
	}
softerr(err, NULL);
}


static void ado_selit(Menuhdr *mh, SHORT hitid)
/* What do we do in response to a pull-down selection? */
{
	switch(hitid)
		{
		case MOV_IN__PUL:	 /* ease */
			vs.ado_ease = !vs.ado_ease;
			break;
		case MOV_OUT_PUL:	 /* ease out */
			vs.ado_ease_out = !vs.ado_ease_out;
			break;
		case MOV_STI_PUL:
			vs.ado_tween = !vs.ado_tween;
			break;
		case MOV_PIN_PUL:
			vs.ado_pong = !vs.ado_pong;
			break;
		case MOV_REV_PUL:
			vs.ado_reverse = !vs.ado_reverse;
			break;
		case MOV_COM_PUL:
			vs.ado_complete = !vs.ado_complete;
			break;
		case ELE_FLI_PUL:		/* frame */
			vs.ado_source = OPS_SCREEN;
			break;
		case ELE_CEL_PUL:		/* cel */
			vs.ado_source = OPS_THECEL;
			break;
		case ELE_POL_PUL:
			vs.ado_source = OPS_POLY;
			break;
		case ELE_SPL_PUL:
			vs.ado_source = OPS_SPLINE;
			break;
		case ELE_TWE_PUL:
			vs.ado_source = OPS_TWEEN;
			break;
		case ELE_STW_PUL:		/* Set Tween */
		{
			tween_menu(FALSE);
			vs.ado_source = OPS_TWEEN;
			break;
		}
		case ELE_SCE_PUL:		/* Set Cel */
			go_nodraw_cel_menu();
			vs.ado_source = OPS_THECEL;
			break;
		case ELE_OUT_PUL:		/* outline */
			vs.ado_outline = !vs.ado_outline;
			break;
		default:
			goto HMP;
		}
	return;
HMP:
	hide_mp();
	switch(hitid)
		{
		case PRE_CLE_PUL:
			ado_clear_all();
			break;
		case PRE_PUL_PUL:
			auto_pull_back();
			break;
		case PRE_SPI_PUL:
			auto_spin();
			break;
		case PRE_TWI_PUL:
			auto_twirl();
			break;
		case PRE_WHI_PUL:
			auto_whirl();
			break;
		case PRE_SPS_PUL:
			auto_spin_small();
			break;
		case PRE_SQU_PUL:
			auto_squash();
			break;
		case PRE_FIL_PUL:
			go_files(8);
			a3d_disables();
			break;
		case PRE_QUI_PUL:
			mh_gclose_code(mh, Err_abort);
			break;
		}
	show_mp();
}

/**************/

static Pentool ado_mouse_ptool = PTOOLINIT1(
	NONEXT,
	empty_str,		/* real name filled in later */
	PTOOL_OPT,
	0,
	NULL,
	NO_SUBOPTS,
	NULL,
	ado_mouse_ptfunc,
	&plain_ptool_cursor,
	NULL, /* on install */
	NULL /* on remove */
);

static Errcode ado_mouse_ptfunc(Pentool *pt, Wndo *w)
/* Hide menus and then go move things around with the mouse above 
 * or abort if a right click */
{
	(void)pt;
	(void)w;

	hide_mp();
	ado_mouse_move();
	show_mp();
	return Success;
}

static Errcode get_a3d_state(void)
/* retrieve optics state from temp file */
{
ado_clear();
if (pj_exists(optics_name))
	{
	return(eload_a3d(optics_name));
	}
return(Success);
}

static Errcode set_a3d_state(void)
/* save optics state to temp file */
{
Errcode err;

err = save_a3d(optics_name);
ado_free_trans();
return(err);
}


void move_along(Button *m)
/* response to 'continue move' */
{
struct ado_setting *as;
LLpoint *first, *last;
Errcode err;
Poly wpoly;

	clear_struct(&wpoly);
	if ((err = do_move_along()) >= Success)
	{
		if (got_path)	/* fold path into move op */
		{
			if (!vs.pa_closed)	
			{
				if (load_a_poly(ppoly_name, &wpoly) >= 0)
				{
					as = vs.move3.next;
					first = wpoly.clipped_list;
					last = slist_el((Slnode *)first, wpoly.pt_count-1);
					as->move.x += last->x - first->x;
					as->move.y += last->y - first->y;
					as->move.z += last->z - first->z;
					free_polypoints(&wpoly);
				}
			}
		}
	}
	softerr(err, "opt_cont");
	draw_buttontop(m);
}

/* one shot structure that's effectively the header to a .opt file */
struct magic_moves
	{
	SHORT magic;
	SHORT moves;
	};

static Errcode load_a3d(char *title)

/* Load up transformation stack from some file somebody must have liked
   sometime... */
{
Errcode err;
struct magic_moves mm;
Jfile fd;
int i;
struct ado_setting *as;

	ado_clear();
	if((fd = pj_open(title, 0)) == JNONE)
		return(pj_ioerr());
	if (pj_read(fd, &mm, (long)sizeof(mm) ) < sizeof(mm) )
	{
		goto jio_error;
	}
	if (mm.magic != A3D_MAGIC)
	{
		err = Err_bad_magic;
		goto error;
	}
	as = NULL;
	i = mm.moves;
	while (--i >= 0)
	{
		if (pj_read(fd, &vs.move3, (long)sizeof(vs.move3) ) < sizeof(vs.move3) )
		{
			goto jio_error;
		}
		vs.move3.next = as;
		if (i != 0)
		{
			if ((err = do_move_along()) < Success)
				goto error;
		}
		as = vs.move3.next;
	}
	err = 0;
	goto done;

jio_error:
	err = pj_ioerr();
error:
done:
	pj_close(fd);
	return(err);
}

static Errcode well_save_a3d(char *title)
/* Try and save the transformation stack */
{
struct magic_moves mm;
Jfile fd;
int i;
struct ado_setting *as;

	mm.magic = A3D_MAGIC;
	i = mm.moves = slist_len((Slnode *)&vs.move3);
	if ((fd = pj_create(title,JWRITEONLY)) == JNONE)
	{
 		return(cant_create(pj_ioerr(),title));
	}
	if (pj_write(fd, &mm, (long)sizeof(mm)) < sizeof(mm) )
	{
		truncated(title);
		goto jio_error;
	}
	while (--i >= 0)
	{
		as = slist_el((Slnode *)&vs.move3, i);
		if (pj_write(fd, as, (long)sizeof(vs.move3) ) < sizeof(*as) )
		{
			truncated(title);
			goto jio_error;
		}
	}

	pj_close(fd);
	return(0);
jio_error:
	pj_close(fd);
	return(pj_ioerr());
}

static Errcode save_a3d(char *title)
/* Save current transformation stack */
{
Errcode err;

	if((err = well_save_a3d(title)) < 0)
		pj_delete(title);		/* delete partial save */
	return(err);
}

static Errcode eload_a3d(char *name)
/* load optics state from file and report error */
{
	return(cant_load(load_a3d(name),name));
}

static char opt_suff[] = ".OPT";

void qload_a3d(void)
/* Put up file requestor and then probably load up optics file */
{
char *title;
char buf[50];

	if ((title = vset_get_filename(stack_string("load_opt",buf),
								  opt_suff,load_str,OPTICS_PATH,
								  NULL,0))!=NULL)
	{
		eload_a3d(title);
	}
}

void qsave_a3d(void)
/* Put up file requestor and then probably save optics file. */
{
char *title;
char buf[50];

	if ((title = vset_get_filename(stack_string("save_opt",buf),
									opt_suff,save_str,OPTICS_PATH,
									NULL,1))!=NULL)
	{
		if(overwrite_old(title))
			save_a3d(title);
	}
}


void mview_path(void)
/* just let em see the path without changing it */
{
Marqihdr mh;
Poly poly;

	clear_struct(&poly);
	vinit_marqihdr(&mh,0,1);
	hide_mp();
	if (load_a_poly(ppoly_name, &poly) >= 0)
	{
		save_undo();
		curveflag = (vs.ado_path == 0);
		is_path = 1;
		marqi_poly(&mh, &poly,vs.pa_closed);
		is_path = 0;
		curveflag = 0;
		wait_click();
		zoom_unundo();
		free_polypoints(&poly);
	}
	show_mp();
}

void edit_path(void)
/* let 'em move around the points on an existing path */
{
Poly wpoly;

	clear_struct(&wpoly);
	hide_mp();
	if (load_a_poly(ppoly_name, &wpoly) >= 0)
	{
		curveflag = (vs.ado_path == 0);
		is_path = 1;
		move_poly_points(&wpoly, vs.pa_closed);
		is_path = 0;
		curveflag = 0;
		save_poly(ppoly_name, &wpoly);
		free_polypoints(&wpoly);
	}
	show_mp();
}

static Errcode do_move_along(void)
/* Duplicate top of transformation stack. */
{
struct ado_setting *as;

make_rot_op();
if ((as = pj_malloc(sizeof(*as)))==NULL)
	return(Err_no_memory);
pj_copy_structure(&vs.move3,as,sizeof(*as));
vs.move3.next = as;
return(Success);
}

static void a3d_check_el(Boolean *no_poly, Boolean *no_tween)
/* make sure that the optics element exists.  If it doesn't set it to
 * the Flic */
{
Boolean np, nt;

	if (thecel == NULL)
		{
		if (vs.ado_source == OPS_THECEL)
			vs.ado_source = OPS_SCREEN;
		}
	*no_poly = np = !pj_exists(poly_name);
	if (np)
		{
		if (vs.ado_source == OPS_SPLINE || vs.ado_source == OPS_POLY)
			vs.ado_source = OPS_SCREEN;
		}
	*no_tween = nt = !got_tween();
	if (nt)
		{
		if (vs.ado_source == OPS_TWEEN)
			vs.ado_source = OPS_SCREEN;
		}
}


static Boolean do_a3dpull(Menuhdr *mh)
/* set disable flags and asterisks in items. */
{
Boolean no_poly, no_tween;

	a3d_check_el(&no_poly, &no_tween);
	set_pul_disable(mh, ELE_CEL_PUL, thecel==NULL);
	set_pul_disable(mh, ELE_SPL_PUL, no_poly);
	set_pul_disable(mh, ELE_POL_PUL, no_poly);
	set_pul_disable(mh, ELE_TWE_PUL, no_tween);
	pul_xflag(mh,MOV_STI_PUL, !vs.ado_tween);
	pul_xflag(mh,MOV_PIN_PUL, vs.ado_pong);
	pul_xflag(mh,MOV_IN__PUL, vs.ado_ease);
	pul_xflag(mh,MOV_OUT_PUL, vs.ado_ease_out);
	pul_xflag(mh,MOV_REV_PUL, vs.ado_reverse);
	pul_xflag(mh,MOV_COM_PUL, vs.ado_complete);
	pul_xflag(mh,ELE_CEL_PUL, vs.ado_source == OPS_THECEL);
	pul_xflag(mh,ELE_FLI_PUL, vs.ado_source == OPS_SCREEN);
	pul_xflag(mh,ELE_SPL_PUL, vs.ado_source == OPS_SPLINE);
	pul_xflag(mh,ELE_POL_PUL, vs.ado_source == OPS_POLY);
	pul_xflag(mh,ELE_TWE_PUL, vs.ado_source == OPS_TWEEN);
	pul_xflag(mh,ELE_OUT_PUL, vs.ado_outline);
	return(menu_dopull(mh));
}
static Boolean do_a3d_keys(void)
{
 	if(check_esc_abort())
		return(TRUE);
	return(common_header_keys());
}
void go_ado(void)
/* Lets go to the optics editor folks! */
{
Menuhdr tpull;
char optics_str[16];		/* The word optics in local language */
void *ss;
Pentool *optool;
Boolean no_poly, no_tween;


	stack_string("optics_str",optics_str);
	unzoom();
	a3d_check_el(&no_poly, &no_tween);	/* Need this before get_a3d_state */
	get_a3d_state();
	hide_mp();
	if(load_a3d_panel(&ss) < Success)
		goto error;

	arrange_a3d_menu();
	clip_tseg();
	fliborder_on();

	optool = vl.ptool;
	ado_mouse_ptool.ot.name = optics_str;
	set_curptool(&ado_mouse_ptool);
	menu_to_quickcent(&a3d_menu);
	if (load_soft_pull(&tpull, 10, "optics", A3DP_MUID,
		ado_selit, do_a3dpull) >= Success)
	{
		do_menuloop(vb.screen,&a3d_menu,NULL,&tpull,do_a3d_keys);
		smu_free_pull(&tpull);
	}
	smu_free_scatters(&ss);
	restore_pentool(optool);
error:
	show_mp();
	set_a3d_state();
	rezoom();
}

