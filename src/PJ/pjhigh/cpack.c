
/* cpack.c - routine to compress an arbitrary # of colors to an
   arbitrary smaller number of colors.  Works pretty cool except
   if you have duplicate colors in the source it sometimes hangs.
   (So filter out duplicate colors before calling pack_ctable... ) */

#include "memory.h"
#include "aaconfig.h"
#include "wndo.h"
#include "cmap.h"



static int usedc;


static int find_newc(int freec, Rgb3 *ctab, int threshold, 
					 Rgb3 *lsource, long lscount)
/* Do a pack with a given threshold */
{
int closestix;
long new;
int dif;

new = 0;
while (new < freec  && --lscount >= 0)
	{
	if (usedc > 0)
		{
		closestix = closestc(lsource,ctab,usedc);
		dif = color_dif(ctab+closestix, lsource);
		}
	if (usedc == 0 || dif > threshold)
		{
		new++;
		ctab[usedc] = *lsource;
		usedc++;
		}
	++lsource;
	}
return(new);
}

void pack_ctable(Rgb3 *source,	/* Source - array scount long of rgb triples */
		    LONG scount,	 /* source size */
		    Rgb3 *dest, /* Destination cmap  MUST be COLORS long even if
		  			      * dcount less.  Duplicate colors here sometimes hang
						    up this routine */
		    int dcount ) /* Dest size (will be packed to this many_ */

/* Pack colors with threshold streaming algorithm.  First calculate
   the smallest threshold that yields less colors than want in
   result.  Stream out at this threshold.  Then go to a finer threshold
   to fill out any remaining spaces in result */
{
int threshold;	/* how far away is closest color before put in this one? */
int newused;			/* # of VGA colors we just ate... */
int i;
int hiwater, lowater;

	if (scount <= dcount)	/* if will fit in dest copy it */
	{
		copy_mem(source, dest, scount*sizeof(Rgb3));
		return;
	}

	/* else pack it: set first guess for threshold */

	if (dcount < 128)
		hiwater = (RGB_MAX-1)*(RGB_MAX-1);
	else
		hiwater = 800;
	lowater = 0;
	threshold = 50*8;

	/* find smallest threshold that's ok... */

	for (i=0;;++i)
	{
		/* and all colors are free to start */
		usedc = 0;
		newused = find_newc(dcount,dest,threshold,source,scount);
		if (newused >= dcount)
			lowater = threshold;
		else
			hiwater = threshold;
		threshold = (hiwater+lowater)/2;
		if (hiwater-lowater <= 1)
			break;
	}
	usedc = 0;
	/* do main threshold operation */
	clear_mem(dest, COLORS*sizeof(Rgb3));
	find_newc(dcount, dest, hiwater,source,scount);
	/* fill up left overs with closer thresholds */
	while (usedc < dcount)
	{
		hiwater = hiwater/2;
		find_newc(dcount-usedc, dest, hiwater, source, scount);
	}
	return;
}
