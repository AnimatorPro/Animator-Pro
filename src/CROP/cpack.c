
/* cpack.c - routine to compress an arbitrary # of colors to an
   arbitrary smaller number of colors.  Works pretty cool except
   if you have duplicate colors in the source it sometimes hangs.
   (So filter out duplicate colors before calling pack_cmap... ) */

#include "jimk.h"
#include "flicmenu.h"
#include "cpack.str"

/* This is only tested with RGB values 0-63 like PC VGA.  May have
   some arithmetic overflow problems if go to 0-256 */


static PLANEPTR lsource;	/* points to unpacked histogram */
static long lscount;	/* # of colors left in unpacked histogram */
static int usedc;


pack_cmap(source,scount,dest,dcount)
PLANEPTR source;	/* Source - array scount long of rgb triples */
long scount;	/* source size */
PLANEPTR dest;	/* Destination cmap  MUST be COLORS long even if dcount less.*/
int dcount;		/* Dest size (will be packed to this many_ */
{
int threshold;	/* how far away is closest color before put in this one? */
int newused;			/* # of VGA colors we just ate... */
int i;
char buf[40];
int hiwater, lowater;

/* set first guess for threshold */
if (dcount < 128)
	hiwater = 63*63;
else
	hiwater = 100;
lowater = 0;
threshold = 50;
/* find smallest threshold that's ok... */
if (scount > dcount)	/* if less in dest than source pack it */
	{
	for (i=1;;i++)
		{
		sprintf(buf, cpack_100 /* "Pass %d threshold %d" */, i, threshold);
		stext(buf, 0, 50, sblack, swhite);
		/* and all colors are free to start */
		usedc = 0;
		lsource = source;
		lscount = scount;
		newused = find_newc(dcount,dest,threshold);
		if (newused >= dcount)
			lowater = threshold;
		else
			hiwater = threshold;
		threshold = (hiwater+lowater)/2;
		if (hiwater-lowater <= 1)
			break;
		}
	/* do main threshold operation */
	sprintf(buf, cpack_101 /* "Final Pass threshold %d" */, hiwater);
	stext(buf, 0, 50, sblack, swhite);
	if (threshold != hiwater)
		{
		usedc = 0;
		lsource = source;
		lscount = scount;
		zero_structure(dest, COLORS*3);
		find_newc(dcount, dest, hiwater);
		}
	/* fill up left overs with closer thresholds */
	hiwater = hiwater/2;
	while (hiwater >= 0 && usedc < dcount)
		{
		lsource = source;
		lscount = scount;
		find_newc(dcount-usedc, dest, hiwater);
		hiwater = hiwater/2;
		}
	}
else	/* else just can copy source to dest */
	{
	copy_bytes(source, dest, scount*3);
	}
}

find_newc(freec, cmap,threshold)
int freec;
PLANEPTR cmap;
int threshold;
{
int closestix;
long new;
int dif;
int gval;
long hc;

new = 0;
while (new < freec  && --lscount >= 0)
	{
	if (usedc > 0)
		{
		closestix = closestc(lsource,cmap,usedc);
		dif = color_dif(cmap+(closestix*3), lsource);
		}
	if (usedc == 0 || dif > threshold)
		{
		new++;
		copy_bytes(lsource, cmap+usedc*3, 3);
		usedc++;
		}
	lsource = norm_pointer(lsource+3);
	}
return(new);
}

