/* Spline demonstration  */
/* by Tom Hudson 9/30/88 */
/* Tweaked into fixed point by Jim Kent */

#define OPEN 0
#define CLOSED 1

#include "jimk.h"
#include "poly.h"

#ifdef LATER
typedef long fpoint;
#define FSHIFT 11
#define FP(a,b) ((a)+(b))
#define FM(a,b) ((a)-(b))
#define FT(a,b) (((a)*(b))>>FSHIFT)
#define FD(a,b) (((a)<<FSHIFT)/(b))
#define FVAL(a)	((long)(a)<<FSHIFT)
#define FINT(a) ((int)((a)>>FSHIFT))
#endif LATER

typedef float fpoint;
#define FP(a,b) ((a)+(b))
#define FM(a,b) ((a)-(b))
#define FT(a,b) ((a)*(b))
#define FD(a,b) ((a)/(b))
#define FVAL(a)	((float)(a))
#define FINT(a) ((int)(a))

#define MXS 25

fpoint cx[MXS],cy[MXS],dinx[MXS],doutx[MXS],diny[MXS],douty[MXS];
fpoint tens[MXS],cont[MXS],bias[MXS];
fpoint sh1,sh2,sh3,sh4;

/* Generate a spline that passes through the control points.       */
/* Supply pointers to control point fpoint arrays (knotx/knoty),    */
/* number of knots, pointers to integer output arrays and number   */
/* of interpreted points to generate.				   */

some_spline(poly, dotout, closed, ir)
struct poly *poly;
Vector dotout;
int closed;
int ir;
{
fpoint *newx, *newy;
int ptcount;
LLpoint *p;
int i, knots;

knots = ptcount = poly->pt_count;
#ifdef LATER
ptcount+=5;
#endif LATER
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
do_spline(newx,newy,poly->pt_count,ir,closed,
	dotout);
freemem(newx);
freemem(newy);
}

lines(x,y,count,dotout)
int *x,*y;
int count;
Vector dotout;
{
int tx,ty,lx,ly;

lx = *x++;
ly = *y++;
while (--count > 0)
	{
	tx = *x++;
	ty = *y++;
	cline(tx,ty,lx,ly,dotout);
	lx = tx;
	ly = ty;
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

do_spline(knotx,knoty,knots,interps,type,dotout)
fpoint *knotx,*knoty;
int knots,interps,type;
Vector dotout;
{
fpoint s,delta;
int ix,ipix,igrp,ipt,next;
int x,y;

/* If open spline, set the end tensions to 1.0 */

for (ix=0; ix<knots; ix++)
	tens[ix] = FVAL(0);
if(type==OPEN)
 tens[0]=tens[knots-1]=FVAL(1);

/* Calc step size between key points */

delta=FVAL(1)/FVAL(interps+1);

/* Plug in knot coords at their positions in line table */
/* Also calc vectors for incoming & outgoing for each   */

for(ix=0; ix<knots; ++ix)
 {
 calc_vecs(knotx,knoty,ix,knots);
 (*dotout)(FINT(knotx[ix]), FINT(knoty[ix]));
 }


/* Interpolate all necessary points! */

for(s=delta,ipt=1; s<1.0; s+=delta,++ipt)
 {
 gen_sh_matrix(s);
 for(ix=0,igrp=0; ix<(knots+type-1); ++ix,igrp+=(interps+1))
  {
  next=(ix+1) % knots;
  x = sh1*knotx[ix] + sh2*knotx[next] +
		sh3*doutx[ix] + sh4*dinx[next];
  y = sh1*knoty[ix] + sh2*knoty[next] +
		sh3*douty[ix] + sh4*diny[next];
  (*dotout)(x,y);
  }
 }
}

/* Generate hermite interpolation matrix s*h	 */
/* This is done once for each interpolation step */

gen_sh_matrix(s)
fpoint s;
{
fpoint s2,s3;

s2 = FT(s,s);
s3 = FT(s2,s);
sh1 = FP(FM(FT(FVAL(2),s3) , FT(FVAL(3),s2)) , 1);
sh2 = FP(FT(FVAL(-2),s3) , FT(FVAL(3),s2));
sh3 = FP(FM(s3 , FT(FVAL(2),s2)) , s);
sh4 = FM(s3,s2);
}

/* Calc incoming & outgoing vectors for knot x */

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

