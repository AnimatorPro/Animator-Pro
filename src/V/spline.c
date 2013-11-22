/* spline.c - hermitian splines with tension/continuity/bias.  Always go
   through their control points. */

/* Spline demonstration  */
/* by Tom Hudson 9/30/88 */
/* Tweaked into fixed point by Jim Kent */

#define OPEN 0
#define CLOSED 1

#include "jimk.h"
#include "poly.h"
#include "spline.str"

typedef long fpoint;
#define FSHIFT 8
#define FP(a,b) ((a)+(b))
#define FM(a,b) ((a)-(b))
#define FT(a,b) (((a)*(b))>>FSHIFT)
#define oFD(a,b) (((a)<<FSHIFT)/(b))
#define FVAL(a)	((long)(a)<<FSHIFT)
#define FINT(a) ((int)((a)>>FSHIFT))

#ifdef LATER
typedef float fpoint;
#define FP(a,b) ((a)+(b))
#define FM(a,b) ((a)-(b))
#define FT(a,b) ((a)*(b))
#define oFD(a,b) ((a)/(b))
#define FVAL(a)	((float)(a))
#define FINT(a) ((int)(a))
#endif LATER

extern int pxmin, pxmax, pymin, pymax;


static fpoint
FD(a,b)
fpoint a,b;
{
return(((a<<FSHIFT)+b/2)/b);
}


static fpoint sh1,sh2,sh3,sh4;
static int *lx, *ly;
static fpoint *dinx,*doutx,*diny,*douty;
static fpoint *tens,*cont,*bias;

static fpoint **ftabs[] = 
	{
	&dinx, &doutx, &diny, &douty, &tens, &cont, &bias,
	};

#define FTS (Array_els(ftabs))

static
free_spline_tab()
{
int i;
register fpoint **ff;

for (i=0; i<FTS; i++)
	{
	ff = ftabs[i];
	gentle_freemem(*ff);
	*ff = NULL;
	}
gentle_freemem(lx);
lx = NULL;
gentle_freemem(ly);
ly = NULL;
}

static
aspline_tab(count)
int count;
{
long size;
int i;
register fpoint **ff;

size = count*sizeof(fpoint);
for (i=0; i<FTS; i++)
	{
	ff = ftabs[i];
	if ((*ff = lbegmem(size)) == NULL)
		return(0);
	}
size = (count+1)*sizeof(*lx);
if ((lx = lbegmem(size)) == NULL)
	return(0);
if ((ly = lbegmem(size)) == NULL)
	return(0);
return(1);
}

static
alloc_spline_tab(count)
int count;
{
if (!aspline_tab(count))
	{
	free_spline_tab();
	return(0);
	}
else
	return(1);
}


/* Generate a spline that passes through the control points.       */
/* Supply pointers to control point fpoint arrays (knotx/knoty),    */
/* number of knots, pointers to integer output arrays and number   */
/* of interpreted points to generate.				   */

static
s_spline(poly, dotout, vecout, closed, ir, filledp)
Poly *poly;
Vector dotout,vecout;
int closed;
int ir;
Poly *filledp;
{
fpoint *newx, *newy;
int ptcount;
LLpoint *p;
int i;

ptcount = poly->pt_count;
if ((newx = begmem(ptcount*sizeof(fpoint) )) == NULL)
	return(0);
if ((newy = begmem(ptcount*sizeof(fpoint) )) == NULL)
	{
	freemem(newx);
	return(0);
	}
p = poly->clipped_list;
for (i=0; i<ptcount; i++)
	{
	newx[i] = FVAL(p->x);
	newy[i] = FVAL(p->y);
	p = p->next;
	}
if (alloc_spline_tab(ptcount))
	{
	do_spline(newx,newy,ptcount,ir,closed,
		dotout,vecout, filledp);
	free_spline_tab();
	}
freemem(newx);
freemem(newy);
return(1);
}

some_spline(poly, dotout, vecout, closed, ir)
Poly *poly;
Vector dotout,vecout;
int closed;
int ir;
{
return(s_spline(poly,dotout,vecout,closed,ir,NULL));
}

make_sp_poly(poly, dpoly, closed, ir)
Poly *poly, *dpoly;
int closed, ir;
{
long ptcount;

zero_structure(dpoly, sizeof(*dpoly) );
if (poly->pt_count > 400)
	{
	ir >>= 1;
	if (poly->pt_count > 800)
		ir >>= 1;
	}
ptcount = dpoly->pt_count = (poly->pt_count-!closed)*ir;
if ((dpoly->clipped_list = lbegmem((long)(1+ptcount) * sizeof(LLpoint)))
	== NULL)
	return(0);
linkup_poly(dpoly);
if (s_spline(poly,(Vector)NULL,(Vector)NULL,closed,ir,dpoly))
	{
	return(1);
	}
else
	{
	gentle_freemem(dpoly->clipped_list);
	return(0);
	}
}




filled_spline(poly)
Poly *poly;
{
Poly sp_poly;
int ok = 0;

if (make_sp_poly(poly, &sp_poly, 1, 16))
	{
	poly_grad_dims(&sp_poly);
	ok = fill_concave(&sp_poly);
	freemem(sp_poly.clipped_list);
	}
return(ok);
}

hollow_spline(poly)
Poly *poly;
{
Poly sp_poly;

if (make_sp_poly(poly, &sp_poly, vs.closed_curve, 16))
	{
	sp_poly.closed = vs.closed_curve;
	poly_grad_dims(&sp_poly);
	render_opoly(&sp_poly);
	freemem(sp_poly.clipped_list);
	}
}




/****************************************************************/
/* Generate a spline that passes through the control points.	*/
/* Uses hermite interpolation					*/
/*								*/
/* knotx & knoty: floating point knot positions			*/
/* knots: number of knots					*/
/* linex & liney: output integer array				*/
/* interps: # of interpolated pts between knots			*/
/* type: OPEN or CLOSED						*/
/****************************************************************/

int is_path;

static
do_spline(knotx,knoty,knots,interps,type,dotout,vecout,filledp)
fpoint *knotx,*knoty;
int knots,interps,type;
Vector dotout;
Vector vecout;
Poly *filledp;
{
fpoint s;
int ix,next,tix, fpix;
int x,y;
int dofill;
LLpoint *p;
int lt,lc,lb;
fpoint t,c,b;

/* If open spline, set the end tensions to 1.0 */
dofill = filledp != NULL;
if (dofill)
	p = filledp->clipped_list;
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
/* Also calc vectors for incoming & outgoing for each   */

for(ix=0; ix<knots; ++ix)
 {
 calc_vecs(knotx,knoty,ix,knots);
 }


/* Interpolate all necessary points! */

for (tix=0; tix<=interps; tix++)
 {
 s = FD(FVAL(tix),FVAL(interps));
 gen_sh_matrix(s);
 for(ix=0; ix<(knots+type-1); )
  {
  next=(ix+1) % knots;
  x = FINT(FP(FP(FP(FT(sh1,knotx[ix]),FT(sh2,knotx[next])),FT(sh3,doutx[ix])),\
	  FT(sh4,dinx[next])));
  y = FINT(FP(FP(FP(FT(sh1,knoty[ix]),FT(sh2,knoty[next])),FT(sh3,douty[ix])),\
	  FT(sh4,diny[next])));
  if (tix != 0 && vecout != NULL)
	  (*vecout)(x,y,lx[ix],ly[ix],dotout);
  if (dofill)
  	{
	fpix = ix*interps + tix;
	p[fpix].x = x;
	p[fpix].y = y;
	p[fpix].z = 0;
	}
  lx[ix] = x;
  ly[ix] = y;
  ix += 1;
  }
 }
}

/* Generate hermite interpolation matrix s*h	 */
/* This is done once for each interpolation step */

static
gen_sh_matrix(s)
fpoint s;
{
fpoint s2,s3;

s2 = FT(s,s);
s3 = FT(s2,s);
sh1 = FP(FM(FT(FVAL(2),s3) , FT(FVAL(3),s2)) , FVAL(1));
sh2 = FP(FT(FVAL(-2),s3) , FT(FVAL(3),s2));
sh3 = FP(FM(s3 , FT(FVAL(2),s2)) , s);
sh4 = FM(s3,s2);
}

/* Calc incoming & outgoing vectors for knot x */

static
calc_vecs(knotx,knoty,x,knots)
fpoint *knotx,*knoty;
int x,knots;
{
fpoint c1,c2,dxi,dxo,dyi,dyo,tc1,tc2;
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

