#include "jimk.h"
#include "errcodes.h"
#include "marqi.h"
#include "memory.h"
#include "poly.h"
#include "polyrub.h"
#include "zoom.h"

static void rub_wpoints(Poly *poly, Pixel pt_color, Pixel pt1_color)
{
LLpoint *p;
int i;
Pixel icols[2];

	p = poly->clipped_list;
	i = poly->pt_count;
	icols[1] = pt1_color;
	while (--i >= 0)
	{
		tblit_image(&circ3_image,icols,vb.pencel,p->x-2,p->y-2);
		rect_zoom_it(p->x-2,p->y-2, 5, 5);
		icols[1] = pt_color;
		p = p->next;
	}
}

static void undraw_wpoints(Poly *poly)
{
LLpoint *p;
int i;

	p = poly->clipped_list;
	i = poly->pt_count;
	while (--i >= 0)
	{
		zoom_undo_rect(p->x-2,p->y-2,5,5);
		p = p->next;
	}
}

void rub_poly_points(Poly *p, Boolean closed,
	Pixel dit_color, Pixel dot_color, Pixel pt_color, Pixel pt1_color)
{
Marqihdr mh;

cinit_marqihdr(&mh,dit_color,dot_color,TRUE);
marqi_poly(&mh, p, closed);
rub_wpoints(p,pt_color,pt1_color);
}

static void unrub_poly_points(Poly *p, Boolean closed)
{
Marqihdr mh;

cinit_marqihdr(&mh,0,0,TRUE);
undo_poly(&mh, p, closed);
undraw_wpoints(p);
}

void move_poly_points(Poly *poly, Boolean closed)
{
LLpoint restore;
LLpoint *lp;
long dist;


	if (poly->pt_count < 1)
		return;
	save_undo();
	for (;;)
	{
		rub_poly_points(poly, closed,
			vs.ccolor, vs.ccolor, vb.screen->SBRIGHT, vb.screen->SBRIGHT);
		wait_wndo_input(ANY_CLICK);
		if (JSTHIT(MBRIGHT|KEYHIT) || 
			(lp = closest_point(poly, icb.mx, icb.my, &dist)) == NULL)
		{
			unrub_poly_points(poly,closed);
			return;
		}
		pj_copy_structure(lp, &restore, sizeof(restore) );
		for (;;)
		{
			wait_any_input();
			unrub_poly_points(poly,closed);
			if (JSTHIT(MBRIGHT|KEYHIT))
			{
				pj_copy_structure(&restore, lp, sizeof(*lp) );
				break;
			}
			if (JSTHIT(MBPEN))
			{
				break;
			}
			lp->x = icb.mx;
			lp->y = icb.my;
			rub_poly_points(poly, closed,
				vs.ccolor, vs.ccolor, vb.screen->SBRIGHT, vb.screen->SBRIGHT);
		}
	}
}

static void marqi_mpl_polys(Mpl_data *data, Marqihdr *mh)
{
int i;
Poly *poly;

for (i=0; i<data->pcount; i++)
	{
	poly = data->polys[i];
	cinit_marqihdr(mh,data->dit_colors[i],data->dot_colors[i],TRUE);
	marqi_poly(mh,poly,vs.closed_curve);
	}
}

static void unmarqi_mpl_polys(Mpl_data *data, Marqihdr *mh)
{
int i;
Poly *poly;

for (i=0; i<data->pcount; i++)
	{
	poly = data->polys[i];
	undo_poly(mh,poly, vs.closed_curve);
	}
}

Errcode move_polys_loop(Mpl_data *data, int *pdx, int *pdy)
{
Marqihdr mh;
int ix,iy;
int lx,ly;
Poly *poly;
int i;


lx = ix = icb.mx;
ly = iy = icb.my;
for (;;)
	{
	marqi_mpl_polys(data,&mh);
	wait_input(MBPEN|MBRIGHT|MMOVE);
	unmarqi_mpl_polys(data,&mh);
	for (i=0; i<data->pcount; i++)
		{
		poly = data->polys[i];
		offset_poly(poly,icb.mx-lx,icb.my-ly,0);
		}
	if(JSTHIT(MBRIGHT))
		{
		for (i=0; i<data->pcount; i++)
			{
			poly = data->polys[i];
			offset_poly(poly,ix-icb.mx,iy-icb.my,0);
			}
		*pdx = *pdy = 0;
		return(Err_abort);
		}
	if (JSTHIT(MBPEN))
		{
		*pdx = icb.mx-ix;
		*pdy = icb.my-iy;
		return(Success);
		}
	lx = icb.mx;
	ly = icb.my;
	}
}

Errcode rub_move_poly(Poly *poly, Pixel color, int *pdx, int *pdy)
{
Rcel_save oundo;
Errcode err;
Mpl_data mpl;

if ((err = report_temp_save_rcel(&oundo, undof)) < Success)
	return(err);
save_undo();

mpl.pcount = 1;
mpl.polys = &poly;
mpl.dit_colors = mpl.dot_colors = &color;
err = move_polys_loop(&mpl, pdx, pdy);

zoom_unundo();
report_temp_restore_rcel(&oundo, undof);
return(err);
}

static void tscale_poly(Poly *source, Poly *dest, int p, int q, int x, int y)
{
int pcount;
LLpoint *sp, *dp;

pcount = dest->pt_count;
sp = source->clipped_list;
dp = dest->clipped_list;
while (--pcount >= 0)
	{
	dp->x = x + (sp->x - x)*p/q;
	dp->y = y + (sp->y - y)*p/q;
	dp->z = sp->z;
	sp = sp->next;
	dp = dp->next;
	}
}

static Errcode
rub_sz_polys(Mpl_data *mpl, Poly **origs, int pcount, int *pp, int *pq)
/* Rubberband out a circle.  Then rubberband polygons stretching and
  shrinking them using mouse position relative to initial circle to
  scale. */
{
int p,q;
Errcode err;
Short_xy cent;
short diam;
Marqi_circdat cd;
Marqihdr mh;
Poly **dests;
Pixel cco;
int i;

cco = vb.screen->SWHITE;
if ((err = get_rub_circle(&cent,&diam,cco)) < Success)
	return(err);
if (diam < 2)	/* make action reasonable on small circles */
	diam = 2;
q = diam;

if ((err = init_circdat(&cd,cco)) < Success)
	return(err);
doval(cent.x,cent.y,q,
	vb.pencel->aspect_dx, vb.pencel->aspect_dy,
	cd.mh.pdot,&cd.mh,NULL,NULL,FALSE);
save_undo();
dests = mpl->polys;
for (;;)
	{
	p = calc_distance(cent.x<<1,cent.y<<1,icb.mx<<1,icb.my<<1);
	for (i=0; i<pcount; i++)
		tscale_poly(origs[i],dests[i],p,q,cent.x,cent.y);
	savedraw_circle(&cd, &cent, p);
	marqi_mpl_polys(mpl,&mh);
	wait_input(MBPEN|MBRIGHT|MMOVE);
	unmarqi_mpl_polys(mpl,&mh);
	restore_circle(&cd, &cent, p);
	if (JSTHIT(MBPEN))
		{
		err = Success;
		break;
		}
	if (JSTHIT(MBRIGHT))
		{
		err = Err_abort;
		break;
		}
	}
*pp = p;
*pq = q;
pj_free(cd.save);
return(err);
}

Errcode rub_size_polys(Mpl_data *mpl, int *pp, int *pq)
/* Allocate clones of polys in mpl-data (up to RSPMAX), and
   then call rub_sz_polys.  If operation aborted restore
   mpl-data polys from clones and pass back error code.  
   Else free clones. */
{
#define RSPMAX 2
Errcode err;
Poly opolys[RSPMAX];
Poly *origs[RSPMAX];
int pcount;
int i;

pcount = mpl->pcount;
if (pcount > RSPMAX)
	return(Err_nogood);
clear_mem(opolys, sizeof(opolys));
for (i=0; i<pcount; i++)
	{
	if ((err = update_poly(mpl->polys[i], &opolys[i])) < Success)
		{
		while (--i >= 0)		/* free one's already allocated */
			free_polypoints(&opolys[i]);
		return(err);
		}
	origs[i] = &opolys[i];
	}
err = rub_sz_polys(mpl,origs,pcount,pp,pq);
for (i=0; i<pcount; i++)
	{
	if (err < Success)	/* restore old poly */
		{
		free_polypoints(mpl->polys[i]);
		*(mpl->polys[i]) = opolys[i];
		}
	else				/* free backup poly */
		{
		free_polypoints(&opolys[i]);
		}
	}
return(err);
#undef RSPMAX
}

Errcode rub_keep_star_type(Poly *p, int startype, int color, int *ptheta,
	int *prad, int *prad2)
/* rubberband out star-class shape.  Keep resulting poly in p (which
   must be initialized) */
{
Rcel_save oundo;
Errcode err;

if ((err = report_temp_save_rcel(&oundo, undof)) < Success)
	return(err);
save_undo();

if (startype == WP_ELLIPSE)
	{
	err = oval_loop(p, color,prad,prad2,ptheta);
	}
else
	{
	err = polystar_loop(p, startype, color, color,
		icb.mx, icb.my, ptheta, prad);
	}

zoom_unundo();
report_temp_restore_rcel(&oundo, undof);
return(err);
}

#ifdef SLUFFED
Errcode rub_star_type(int startype, 
	int color, int *ptheta, int *prad, int *prad2)
/* Rubberband out a star-class shape and return the final radius and angle */
{
Poly p;
Errcode err;

clear_struct(&p);
wait_click();
if (JSTHIT(MBPEN))
	{
	err = rub_keep_star_type(&p, startype, color, ptheta, prad, prad2);
	free_polypoints(&p);
	}
else
	err = Err_abort;
return(err);
}
#endif /* SLUFFED */

static Errcode shape_loop(Poly *poly
, LLpoint *this, Pixel oncolor, Pixel offcolor)
{
Marqihdr mh;
LLpoint *last;

	cinit_marqihdr(&mh,oncolor,offcolor,TRUE);
	for (;;)
	{
		wait_input(MMOVE|MBPUP);
		if(ISDOWN(MBPEN))
		{
			last = this;
			if ((this = new_poly_point(poly)) == NULL)
				return(Err_no_memory);	
			pj_cline(last->x, last->y, this->x, this->y, 
				  mh.pdot, &mh);
		}
		else
			break;
	}
	return(Success);
}

Errcode get_rub_shape(Poly *poly,Pixel ocolor, Pixel offcolor)
/* create poly points as long as pen is down requires save undo first */
{
Errcode err;
LLpoint *this;

	free_polypoints(poly);
	poly->polymagic = POLYMAGIC;

	if((this = new_poly_point(poly)) != NULL)
		err = shape_loop(poly, this, ocolor, offcolor);
	else
		err = Err_no_memory;
	if(err < Success)
		free_polypoints(poly);
	zoom_unundo();
	return(err);
}
