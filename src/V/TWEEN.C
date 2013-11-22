
/* tween.c - stuff to handle what happens when use pushes 'tween' under
   the poly submenu inside the draw tools menu.   Given two polygons
   generate  polygons that are interpolated beTWEEN them. */

#include "jimk.h"
#include "poly.h"
#include "tween.str"

extern int pxmin, pxmax, pymin, pymax;

extern Poly working_poly;

extern char curveflag;


static Poly *poly0, *poly1;

static
tween1(ix, intween, scale)
int ix, intween, scale;
{
Poly dpoly;
Vertex *v, *vbuf;
int ptcount;
LLpoint *d;
int ok;

dpoly.pt_count  = ptcount = tween_pt_count(poly0, poly1, scale);
if ((dpoly.clipped_list = d = begmem(ptcount*sizeof(LLpoint))) == NULL)
	return(0);
if ((v = vbuf = begmem(ptcount*sizeof(Vertex))) == NULL)
	{
	freemem(d);
	return(0);
	}
dpoly.closed = poly0->closed = poly1->closed = vs.closed_curve||vs.fillp;
xform_to_ado_poly(poly0, poly1, v, ptcount, scale);
linkup_poly(&dpoly);
while (--ptcount>=0)
	{
	d->x = v->x;
	d->y = v->y;
	d->z = v->z;
	d = d->next;
	v += 1;
	}
ok = render_poly(&dpoly, vs.fillp);
freemem(vbuf);
freemem(dpoly.clipped_list);
return(ok);
}

edit_poly()
{
Poly opoly;

hide_mp();
if (load_working_poly())
	{
	move_poly_points();
	finish_polyt(vs.fillp);
	}
draw_mp();
}

edit_curve()
{
curveflag = 1;
edit_poly();
curveflag = 0;
}

static
force_poly(name)
char *name;
{
if (!jexists(name))
	{
	if (!jcopyfile(poly_name, name))
		return(0);
	}
return(1);
}


static
adjust_p(name)
char *name;
{
if (!force_poly(name))
	return;
if (load_poly(name))
	{
	move_poly_points();
	save_poly(name, &working_poly);
	poly_nopoints(&working_poly);
	}
}

static
adjust_start()
{
adjust_p(poly1_name);
}

static
adjust_end()
{
adjust_p(poly2_name);
}

static
set_start()
{
if (jcopyfile(poly_name,poly1_name))
	adjust_start();
}

static
set_end()
{
if (jcopyfile(poly_name,poly2_name))
	adjust_end();
}

static
end_to_start()
{
jcopyfile(poly2_name, poly1_name);
}

do_tween()
{
Poly opoly;
int omulti;

if (!force_poly(poly1_name))
	return;
if (!force_poly(poly2_name))
	return;
if (load_poly(poly1_name))
	{
	copy_structure(&working_poly, &opoly, sizeof(opoly) );
	zero_structure(&working_poly, sizeof(working_poly) );
	if (load_poly(poly2_name))
		{
		poly0 = &opoly;
		poly1 = &working_poly;
		omulti = vs.multi;
		vs.multi = 1;
		if (push_inks())
			{
			if (ink_push_cel())
				{
				doauto(tween1);
				ink_pop_cel();
				}
			else
				noroom();
			pop_inks();
			}
		else
			noroom();
		vs.multi = omulti;
		poly_nopoints(&working_poly);
		}
	poly_nopoints(&opoly);
	}
}


static
char *tween_choices[] =
	{
	tween_100 /* "Set start position" */,
	tween_101 /* "Adjust start" */,
	tween_102 /* "Set end position" */,
	tween_103 /* "Adjust end" */,
	tween_104 /* "End to start" */,
	tween_105 /* "Do Tween" */,
	tween_106 /* "Exit menu" */,
	};

extern close_menu();

static
Vector tween_feelers[] = 
	{
	set_start,
	adjust_start,
	set_end,
	adjust_end,
	end_to_start,
	do_tween,
	close_menu,
	};



tween_poly()
{
hide_mp();
if (jexists(poly_name))
	{
	qmenu(tween_107 /* "Tweening options" */, 
		tween_choices, Array_els(tween_choices), tween_feelers);
	}
draw_mp();
}

tween_curve()
{
curveflag = 1;
tween_poly();
curveflag = 0;
}

