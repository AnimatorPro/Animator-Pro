/* spline.c - hermitian splines with tension/continuity/bias.  Always go
   through their control points. */

/* Spline demonstration  */
/* by Tom Hudson 9/30/88 */
/* Tweaked into fixed point by Jim Kent */
/* setup for ansi c by Peter Kennard */

#define OPEN 0
#define CLOSED 1

#include "jimk.h"
#include "errcodes.h"
#include "fixpoint.h"
#include "imath.h"
#include "memory.h"
#include "poly.h"
#include "render.h"

int is_path;

static fixpoint sh1,sh2,sh3,sh4;
static int *lx, *ly;
static fixpoint *dinx,*doutx,*diny,*douty;
static fixpoint *tens,*cont,*bias;
static UBYTE *do_segment;

static fixpoint **ftabs[] =
	{
	&dinx, &doutx, &diny, &douty, &tens, &cont, &bias,
	};

static void
do_spline(fixpoint *knotx, fixpoint *knoty, int knots, int interps, int type,
		dotout_func dotout, void *dotdat,
		void (*vecout)(SHORT, SHORT, SHORT, SHORT, dotout_func, void *),
		Poly *polyout, int moving_point_ix, int invert_segment);

static void gen_sh_matrix(fixpoint s);

#define FTS (Array_els(ftabs))

static void free_spline_tab(void)
{
int i;
register fixpoint **ff;

for (i=0; i<FTS; i++)
	{
	ff = ftabs[i];
	pj_gentle_free(*ff);
	*ff = NULL;
	}
pj_gentle_free(lx);
lx = NULL;
pj_gentle_free(ly);
ly = NULL;
pj_gentle_free(do_segment);
do_segment = NULL;
}

static Errcode alloc_spline_tab(int count)
{
long size;
int i;
register fixpoint **ff;

	size = count*sizeof(fixpoint);
	for (i=0; i<FTS; i++)
	{
		ff = ftabs[i];
		if ((*ff = pj_malloc(size)) == NULL)
			goto error;
	}
	size = (count+1)*sizeof(*lx);
	if ((lx = pj_malloc(size)) == NULL)
		goto error;
	if ((ly = pj_malloc(size)) == NULL)
		goto error;
	if ((do_segment = pj_malloc(count+1)) == NULL)
		goto error;
	return(Success);
error:
	free_spline_tab();
	return(Err_no_memory);
}

static Errcode
s_spline(Poly *polyin,
		dotout_func dotout, void *dotdat,
		void (*vecout)(SHORT, SHORT, SHORT, SHORT, dotout_func, void *),
		int closed, int ir, Poly *polyout,
		int moving_point_ix, int invert_segment)

/* Generate a spline that passes through the control points.	   */
/* Supply pointers to control point fixpoint arrays (knotx/knoty),	*/
/* number of knots, pointers to integer output arrays and number   */
/* of interpreted points to generate.				   */
{
Errcode err;
fixpoint *newx, *newy;
int ptcount;
LLpoint *p;
int i;

	ptcount = polyin->pt_count;

	newy = NULL;
	if ((newx = pj_malloc(ptcount*sizeof(fixpoint) )) == NULL)
		goto nomem_error;
	if ((newy = pj_malloc(ptcount*sizeof(fixpoint) )) == NULL)
		goto nomem_error;

	p = polyin->clipped_list;
	for (i=0; i<ptcount; i++)
	{
		newx[i] = FVAL(p->x);
		newy[i] = FVAL(p->y);
		p = p->next;
	}
	if((err = alloc_spline_tab(ptcount)) < 0)
		goto error;

	do_spline(newx,newy,ptcount,ir,closed,
			  dotout,dotdat,vecout,polyout,moving_point_ix, invert_segment);
	free_spline_tab();

	err = Success;
	goto done;

nomem_error:
	err = Err_no_memory;
done:
error:
	pj_gentle_free(newx);
	pj_gentle_free(newy);
	return(err);
}

int
some_spline(Poly *poly,
		dotout_func dotout, void *dotdat,
		void (*vecout)(SHORT, SHORT, SHORT, SHORT, dotout_func, void *),
		int closed, int ir)
{
LLpoint *p;

if (poly->pt_count == 1)
	{
	p = poly->clipped_list;
	(*dotout)(p->x, p->y, dotdat);
	return(Success);
	}
return(s_spline(poly,dotout,dotdat,vecout,closed,ir,NULL,-1,0));
}

int
partial_spline(Poly *poly,
		dotout_func dotout, void *dotdat,
		void (*vecout)(SHORT, SHORT, SHORT, SHORT, dotout_func, void *),
		int closed, int ir, int moving_point_ix, int invert_seg)
{
	return(s_spline(poly,dotout,dotdat,vecout,closed,ir,NULL,
		moving_point_ix,
		invert_seg));
}


int make_sp_poly(Poly *poly,Poly *dpoly,int closed,int ir)
{
int ptcount = poly->pt_count;
Errcode err = Success;
int open = !closed;

clear_mem(dpoly, sizeof(*dpoly) );
if ((dpoly->pt_count = ptcount = (ptcount-open)*ir + open) > BIG_SHORT)
	return(Err_spline_points);
if ((dpoly->clipped_list =	pj_malloc((long)(ptcount+1) * sizeof(LLpoint)))
	== NULL)	/* alloc one extra to simplify lower level spline loop. */
	return(Err_no_memory);
linkup_poly(dpoly);
if (ptcount == 1)
	{
	*(dpoly->clipped_list) = *(poly->clipped_list);
	}
else
	{
	if ((err = s_spline(poly, NULL, NULL, NULL, closed, ir, dpoly, -1, 0)) < 0)
		{
		pj_gentle_free(dpoly->clipped_list);
		dpoly->clipped_list = NULL;
		}
	}
return(err);
}

Errcode filled_spline(Poly *poly)
{
Poly sp_poly;
Errcode err;

	if((err = make_sp_poly(poly, &sp_poly, 1, 16)) < 0)
		return(err);

	poly_grad_dims(&sp_poly, TRUE);
	err = render_fill_poly(&sp_poly);
	pj_free(sp_poly.clipped_list);
	return(err);
}

Errcode hollow_spline(Poly *poly, Boolean closed)
{
Errcode err;
Poly sp_poly;

	if((err = make_sp_poly(poly, &sp_poly, closed, 16)) < Success)
		return(err);

	poly_grad_dims(&sp_poly, FALSE);
	err = render_opoly(&sp_poly, closed);
	pj_free(sp_poly.clipped_list);
	return(err);
}

static void calc_vecs(fixpoint *knotx,fixpoint *knoty,int x,int knots)
/* Calc incoming & outgoing vectors for knot x */
{
fixpoint c1,c2,dxi,dxo,dyi,dyo,tc1,tc2;
int next,last;

	/* Calc deltas for the points */

	next=(x+1) % knots;

	if(x==0)
	 last=knots-1;
	else
	 last=x-1;

	/* Calc some temps to speed things up */

	dxi=FM(knotx[x],knotx[last]);
	dyi=FM(knoty[x],knoty[last]);
	dxo=FM(knotx[next],knotx[x]);
	dyo=FM(knoty[next],knoty[x]);

	tc1 = FT(FM(FVAL(1),tens[x]) , FM(FVAL(1),cont[x]));
	tc2 = FT(FM(FVAL(1),tens[x]) , FP(FVAL(1),cont[x]));

	c1 = FD(FT(tc1 , (FP(FVAL(1),bias[x]))),FVAL(2));
	c2 = FD(FT(tc2 , (FM(FVAL(1),bias[x]))),FVAL(2));
	dinx[x] = FP(FT(dxi,c1) , FT(dxo,c2));
	diny[x] = FP(FT(dyi,c1) , FT(dyo,c2));

	c1 = FD(FT(tc2 , (FP(FVAL(1),bias[x]))),FVAL(2));
	c2 = FD(FT(tc1 , (FM(FVAL(1),bias[x]))),FVAL(2));
	doutx[x] = FP(FT(dxi,c1) , FT(dxo,c2));
	douty[x] = FP(FT(dyi,c1) , FT(dyo,c2));
}

/* Function: do_spline
 *
 *  Generate a spline that passes through the control points.
 *  Uses hermite interpolation.
 *
 *  knotx, knoty - floating point knot positions.
 *  knots - number of knots.
 *  interps - # of interpolated pts between knots.
 *  type - OPEN or CLOSED.
 *  vecout - draw a line function
 *  polyout - place to put spline as a poly.
 *  moving_point_ix - point that is moving if any.
 *  invert_segment - show just moving part, or just the rest.
 */
static void
do_spline(fixpoint *knotx, fixpoint *knoty, int knots, int interps, int type,
		dotout_func dotout, void *dotdat,
		void (*vecout)(SHORT, SHORT, SHORT, SHORT, dotout_func, void *),
		Poly *polyout, int moving_point_ix, int invert_segment)
{
fixpoint s;
int ix,next,tix, fpix;
int x,y;
int doout;
LLpoint *p;
int lt,lc,lb;
fixpoint t,c,b;
int seg_count;

/* If open spline, set the end tensions to 1.0 */
doout = polyout != NULL;
if (doout)
	p = polyout->clipped_list;
if (is_path)
	{
	lt = vs.pa_tens;
	lc = vs.pa_cont;
	lb = vs.pa_bias;
	}
else
	{
	lt = vs.sp_tens;
	lc = vs.sp_cont;
	lb = vs.sp_bias;
	}
t = FD(FVAL(lt),FVAL(5) );
c = FD(FVAL(lc),FVAL(5) );
b = FD(FVAL(lb),FVAL(5) );
for (ix=0; ix<knots; ix++)
	{
	tens[ix] = t;
	cont[ix] = c;
	bias[ix] = b;
	}
if(type==OPEN)
 tens[0]=tens[knots-1]=FVAL(1);
/* Also calc vectors for incoming & outgoing for each	*/

for(ix=0; ix<knots; ++ix)
 {
 calc_vecs(knotx,knoty,ix,knots);
 }

seg_count = knots+type-1;
/* set up table of segments to calculate/draw */
if (moving_point_ix < 0)
	pj_stuff_bytes(TRUE, do_segment, seg_count);
else
	{
	int do_seg;
	int startix, endix;
	int check_first;

	startix = moving_point_ix-2;
	endix = moving_point_ix+1;
	check_first = (type == CLOSED && seg_count > 2);
	for (ix=0; ix<seg_count; ix++)
		{
		do_seg = (ix >= startix && ix <= endix);
		if (check_first)
			{
			do_seg |= (ix+seg_count >= startix && ix+seg_count <= endix);
			do_seg |= (ix-seg_count >= startix && ix-seg_count <= endix);
			}
		if (invert_segment)
			do_seg = !do_seg;
		do_segment[ix] = do_seg;
		}
	}


/* Interpolate all necessary points! */

for (tix=0; tix<=interps; tix++)
 {
 s = FD(FVAL(tix),FVAL(interps));
 gen_sh_matrix(s);
 for(ix=0; ix<seg_count; )
  {
  if (do_segment[ix])
	  {
	  next=(ix+1) % knots;
	  x = FINT(FP(FP(FP(FT(sh1,knotx[ix]),FT(sh2,knotx[next])),\
		FT(sh3,doutx[ix])), FT(sh4,dinx[next])));
	  y = FINT(FP(FP(FP(FT(sh1,knoty[ix]),FT(sh2,knoty[next])),\
		FT(sh3,douty[ix])),FT(sh4,diny[next])));
	  if (tix != 0 && vecout != NULL)
		  (*vecout)(x,y,lx[ix],ly[ix],dotout,dotdat);
	  if (doout)
		{
		fpix = ix*interps + tix;
		p[fpix].x = x;
		p[fpix].y = y;
		p[fpix].z = 0;
		}
	  lx[ix] = x;
	  ly[ix] = y;
	  }
  ix += 1;
  }
 }
}


static void gen_sh_matrix(fixpoint s)
/* Generate hermite interpolation matrix s*h	 */
/* This is done once for each interpolation step */
{
fixpoint s2,s3;

s2 = FT(s,s);
s3 = FT(s2,s);
sh1 = FP(FM(FT(FVAL(2),s3) , FT(FVAL(3),s2)) , FVAL(1));
sh2 = FP(FT(FVAL(-2),s3) , FT(FVAL(3),s2));
sh3 = FP(FM(s3 , FT(FVAL(2),s2)) , s);
sh4 = FM(s3,s2);
}
