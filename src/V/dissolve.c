/* dissolve.c - misc transition effects */

#include "jimk.h"
#include "fli.h"
#include "flicmenu.h"


venetian_tween(s2, d, tix, tcount)
Vscreen *s2, *d;	/*  source and  destination */
int tix, tcount;   /* ratio of s1/s2 blend */
{
long pcount;
unsigned i;
unsigned cp;
register PLANEPTR sp2, dp1;
unsigned offset;
int ict;

pcount = tix;
pcount *= YMAX;
pcount += tcount/2;
pcount /= tcount;

cp = 0;
ict = 0;
sp2 = s2->p;
dp1 = d->p;
for (i=0; i<pcount; i++)
	{
	offset = cp*BPR;
	copy_structure(sp2+offset, dp1+offset,BPR);
	cp += (unsigned)16;
	if (cp >= YMAX)
		{
		ict += 1;
		cp = ict;
		}
	}
return(1);
}

dissolve_tween(s2, d, tix, tcount)
Vscreen *s2, *d;	/* source and a destination */
int tix, tcount;   /* ratio of s1/s2 blend */
{
unsigned pcount;
unsigned i;
unsigned cp;
register PLANEPTR sp2, dp1;

pcount = (tix * 64000L + tcount/2) / tcount;

cp = 0;
sp2 = s2->p;
dp1 = d->p;
for (i=0; i<pcount; i++)
	{
	dp1[cp] = sp2[cp];
	cp += (unsigned)27*27;
	cp %= (unsigned)64000;
	}
return(1);
}

static PLANEPTR dts,dtd;

/* a horizontal line routine to pass to disk.  I guess I need this...??? 
   Really hate to be clipping all over the place like this.  Sigh. */
static
dthline(y, x0, x1)
register WORD y,x0;
WORD x1;
{
unsigned offset;

/* clip that baby first thing... */
if (y < 0 || y >= YMAX)
	return;
if (x0 >= (int)XMAX)
	return;
if (x1 < 0)
	return;
if (x0 < 0)
	x0 = 0;
if (x1 >= XMAX)
	x1 = XMAX-1;
offset = y;
offset *= BPR;
offset += x0;
copy_bytes(dts+offset,dtd+offset,x1-x0+1);
}


diskd_tween(s2, d, tix, tcount)
Vscreen *s2, *d;	/* sources and a destination */
int tix, tcount;   /* ratio of s1/s2 blend */
{
unsigned pcount;

dts = s2->p;
dtd = d->p;
pcount = ((long)tix * (5+calc_distance(0,0,XMAX/2,YMAX/2))
	+ tcount/2) / tcount;
ccircle(XMAX/2,YMAX/2,pcount,NULL,dthline,TRUE);
}

