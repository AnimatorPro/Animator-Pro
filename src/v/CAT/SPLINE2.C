/*************************/
/* Spline demonstration  */
/* by Tom Hudson 11/4/88 */
/*************************/

#include <graph.h>

#define OPEN 0
#define CLOSED 1

struct videoconfig vc;

/* Spline knot arrays */

float cx[10],cy[10],dinx[10],doutx[10],diny[10],douty[10];
float tens[10],cont[10],bias[10];

/* Tension, continuity, bias set param array */

#define TENSION 0
#define CONTINUITY 1
#define BIAS 2

float sets[3]={0.0,0.0,0.0};
int tcb=TENSION;

/* sh matrix variables */

float sh1,sh2,sh3,sh4;

/* Main section -- draws three splines and allows manipulation */

main()
{
int ix,scx[100],scy[100];
int ch;

/* Set graphics mode */

_setvideomode(_VRES16COLOR);
_getvideoconfig(&vc);

/* Let's do some splines */

dospline:
_clearscreen(_GCLEARSCREEN);

/* Three key-point open spline, 12 interpolations between keys */

cx[0]=100.0;
cx[1]=400.0;
cx[2]=200.0;
cy[0]=100.0;
cy[1]=140.0;
cy[2]=300.0;
tens[0]=tens[1]=tens[2]=sets[0];
cont[0]=cont[1]=cont[2]=sets[1];
bias[0]=bias[1]=bias[2]=sets[2];
do_spline(cx,cy,3,scx,scy,12,OPEN);
plot_line(scx,scy,12*(3-1)+3,1);		/* Open spline */
plot_knots(cx,cy,3,15);

/* Four key-point open spline, 10 interpolations between keys */

cx[0]=600.0;
cx[1]=300.0;
cx[2]=400.0;
cx[3]=100.0;
cy[0]=100.0;
cy[1]=200.0;
cy[2]=300.0;
cy[3]=400.0;
tens[0]=tens[1]=tens[2]=tens[3]=sets[0];
cont[0]=cont[1]=cont[2]=cont[3]=sets[1];
bias[0]=bias[1]=bias[2]=bias[3]=sets[2];
do_spline(cx,cy,4,scx,scy,10,OPEN);
plot_line(scx,scy,10*(4-1)+4,2);		/* Open spline */
plot_knots(cx,cy,4,15);

/* Four key-point closed spline, 8 interpolations between keys */

cx[0]=300.0;
cx[1]=400.0;
cx[2]=300.0;
cx[3]=200.0;
cy[0]=100.0;
cy[1]=200.0;
cy[2]=300.0;
cy[3]=200.0;
tens[0]=tens[1]=tens[2]=tens[3]=sets[0];
cont[0]=cont[1]=cont[2]=cont[3]=sets[1];
bias[0]=bias[1]=bias[2]=bias[3]=sets[2];
do_spline(cx,cy,4,scx,scy,8,CLOSED);		/* Closed spline */
plot_line(scx,scy,8*4+4+CLOSED,3);
plot_knots(cx,cy,4,15);

newmode:
_settextposition(0,0);
printf("Tens Cont Bias + - Reset Quit    T:%4.2f C:%4.2f B:%4.2f",
	sets[0],sets[1],sets[2]);

_settextcolor(4);
switch(tcb)
 {
 case TENSION:
  _settextposition(1,1);
  _outtext("Tens");
  break;
 case CONTINUITY:
  _settextposition(1,6);
  _outtext("Cont");
  break;
 case BIAS:
  _settextposition(1,11);
  _outtext("Bias");
  break;
 }

switch(getch())
 {
 case 't':
 case 'T':
  tcb=TENSION;
  goto newmode;
  break;
 case 'c':
 case 'C':
  tcb=CONTINUITY;
  goto newmode;
  break;
 case 'b':
 case 'B':
  tcb=BIAS;
  goto newmode;
  break;
 case 'r':
 case 'R':
  sets[0]=sets[1]=sets[2]=0.0;
  goto dospline;
  break;
 case 'q':
 case 'Q':
  break;
 case '+':
  sets[tcb]+=0.1;
  goto dospline;
  break;
 case '-':
  sets[tcb]-=0.1;
  goto dospline;
  break;
 default:
  goto newmode;
  break;
 }

_clearscreen(_GCLEARSCREEN);
_setvideomode(_DEFAULTMODE);
}

/* Plot a line from the supplied XY arrays with count and color */

plot_line(x,y,count,color)
int *x,*y,count,color;
{
int ix;

_setcolor(color);
_moveto(x[0],y[0]);
for(ix=1; ix<count; ++ix)
 _lineto(x[ix],y[ix]); 
}

/* Plot the knots from the supplied XY arrays with count and color */

plot_knots(x,y,count,color)
float *x,*y;
int count,color;
{
int ix;

_setcolor(color);
for(ix=0; ix<count; ++ix)
 _setpixel((int)x[ix],(int)y[ix]); 
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

do_spline(knotx,knoty,knots,linex,liney,interps,type)
float *knotx,*knoty;
int knots,*linex,*liney,interps,type;
{
float s,delta;
int ix,ipix,igrp,ipt,next;

/* If open spline, set the end tensions to 1.0 */

if(type==OPEN)
 tens[0]=tens[knots-1]=1.0;

/* Calc step size between key points */

delta=1.0/(float)(interps+1);

/* Plug in knot coords at their positions in line table */
/* Also calc vectors for incoming & outgoing for each   */

for(ix=0; ix<knots; ++ix)
 {
 calc_vecs(knotx,knoty,ix,knots);
 ipix=ix*(interps+1);
 linex[ipix]=(int)knotx[ix];
 liney[ipix]=(int)knoty[ix];
 }

/* If it's a closed spline, put in last point */

if(type==CLOSED)
 {
 ipix=knots*(interps+1);
 linex[ipix]=linex[0];
 liney[ipix]=liney[0];
 }

/* Interpolate all necessary points! */

for(s=delta,ipt=1; s<1.0; s+=delta,++ipt)
 {
 gen_sh_matrix(s);
 for(ix=0,igrp=0; ix<(knots+type-1); ++ix,igrp+=(interps+1))
  {
  next=(ix+1) % knots;
  ipix=igrp+ipt;
  linex[ipix] = sh1*knotx[ix] + sh2*knotx[next] +
		sh3*doutx[ix] + sh4*dinx[next];
  liney[ipix] = sh1*knoty[ix] + sh2*knoty[next] +
		sh3*douty[ix] + sh4*diny[next];
  }
 }
}

/* Generate hermite interpolation matrix s*h	 */
/* This is done once for each interpolation step */

gen_sh_matrix(s)
float s;
{
float s2,s3;

s2=s*s;
s3=s2*s;

sh1 = 2*s3 - 3*s2 + 1;
sh2 = -2*s3 + 3*s2;
sh3 = s3 - 2*s2 + s;
sh4 = s3 - s2;
}

/* Calc incoming & outgoing vectors for knot x */

calc_vecs(knotx,knoty,x,knots)
float *knotx,*knoty;
int x,knots;
{
float c1,c2,dxi,dxo,dyi,dyo,tc1,tc2;
int next,last;

/* Calc deltas for the points */

next=(x+1) % knots;

if(x==0)
 last=knots-1;
else
 last=x-1;

/* Calc some temps to speed things up */

dxi=knotx[x]-knotx[last];
dyi=knoty[x]-knoty[last];
dxo=knotx[next]-knotx[x];
dyo=knoty[next]-knoty[x];

tc1=(1.0-tens[x]) * (1.0-cont[x]);
tc2=(1.0-tens[x]) * (1.0+cont[x]);

c1=(tc1 * (1.0+bias[x]))/2.0;
c2=(tc2 * (1.0-bias[x]))/2.0;
dinx[x] = dxi*c1 + dxo*c2;
diny[x] = dyi*c1 + dyo*c2;

c1=(tc2 * (1.0+bias[x]))/2.0;
c2=(tc1 * (1.0-bias[x]))/2.0;
doutx[x] = dxi*c1 + dxo*c2;
douty[x] = dyi*c1 + dyo*c2;
}

/* That's all, folks! */
s[1]=bias[2]=sets[2];
do_spline(cx,cy,3,scx,scy,12,OPEN);
plot_line(scx,scy,12*(3-1)+3,1);		/* Open spline */
plot_knots(cx,cy,3,15);

/* Four key-point open spline, 10 interpolations between keys */

cx[0]=600.0;
cx[1]=300.0;
cx[2]=400.0;
cx[3]=100.0;
cy[0]=100.0;
cy[1]=200.0;
cy[2]=300.0;
cy[3]=400.0;
tens[0]=tens[1]=tens[2]=tens[3]=sets[0];
cont[0]=cont[1]=cont[2]=cont[3]=sets[1];
bias[0]=bias[1]=bias[2]=bias[3]=sets[2];
do_spline(cx,cy,4,scx,scy,10,OPEN);
plot_line(scx,scy,10*(4-1)+4,2);		/* Open spline */
plot_knots(cx,cy,4,15);

/* Four key-point closed spline, 8 interpolations between keys */

cx[0]=300.0;
cx[1]=400.0;
cx[2]=300.0;
cx[3]=200.0;
cy[0]=100.0;
cy[1]=200.0;
cy[2]=300.0;
cy[3]=200.0;
tens[0]=tens[1]=tens[2]=tens[3]=sets[0];
cont[0]=cont[1]=cont[2]=cont[3]=sets[1];
bias[0]=bias[1]=bias[2]=bias[3]=sets[2];
do_spline(cx,cy,4,scx,scy,8,CLOSED);		/* Closed spline */
plot_line(scx,scy,8*4+4+CLOSED,3);
plot_knots(cx,cy,4,15);

newmode: