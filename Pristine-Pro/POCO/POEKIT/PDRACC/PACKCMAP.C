
/* packcmap.c - routines to compress an arbitrary # of colors to an
 * arbitrary smaller number of colors.
 *
 *	 munged by Ian for pdracces.poe...
 *	 DO NOT LET THIS FIND ITS WAY BACK INTO PJ/CONVERT, IT WON'T WORK!!!
 */

#include <errcodes.h>
#include <syslib.h>
#include <pocolib.h>
#include "packcmap.h"

extern Boolean colors_256;

static Errcode find_newc(int usedc, int freec, Rgb3 *ctab, int threshold,
					 Rgb3 *lsource, long lscount, USHORT *curthresh)
/*****************************************************************************
 * Do a pack with a given threshold
 ****************************************************************************/
{
	Errcode err;
	int 	closestix;
	long	new;
	int 	dif;

	poeprintf(3,12,ptr2ppt("Packing %6ld colors to %4d threshold %4d",0),
			lscount-usedc, freec, threshold);

	new = 0;
	while (--lscount >= 0)
		{
		if (*curthresh > threshold)
			{
			closestix = closestc(lsource,ctab,usedc);
			dif = color_dif(ctab+closestix, lsource);
			if (dif > threshold)
				{
				if (new >= freec) {
					err = Err_overflow;
					goto EXIT;
				}
				new++;
				ctab[usedc] = *lsource;
				usedc++;
				*curthresh = 0;
				}
			else
				*curthresh = dif;
			}
		++curthresh;
		++lsource;
		}

EXIT:

	return(new);
}

Errcode fpack_ctable(
		Rgb3 *source,	/* Source - array scount long of rgb triples */
		LONG scount,	 /* source size */
		Rgb3 *dest, /* Destination cmap  MUST be COLORS long even if
				  * dcount less.  Duplicate colors here sometimes hang
					up this routine */
		int dcount)  /* Dest size (will be packed to this many) */
/*****************************************************************************
 * Pack colors with adaptive threshold streaming algorithm.
 ****************************************************************************/
{
	int threshold;	/* how far away is closest color before put in this one? */
	int newused;			/* # of VGA colors we just ate... */
	int usedc;
	int hiwater;
	int reduction_factor;
	USHORT *curthresh = NULL;
	int ctsize = scount * sizeof(*curthresh);

	if (scount <= dcount)	/* if will fit in dest copy it */
		{
		memcpy(dest, source, scount*sizeof(Rgb3));
		return Success;
		}

	if ((curthresh = malloc(ctsize)) == NULL)
		return(Err_no_memory);

	*dest = *source;		/* copy in the first color */
	usedc = 1;
	threshold = 1024;
							/* Do the first threshold.	If have overflow
							 * set the threshold to 2x and do it again...*/
FINDHI:

	memset(curthresh,0xff,ctsize);
	*curthresh = 0;
	newused = find_newc(usedc, dcount-usedc, dest,
						threshold, source, scount,curthresh);
	if (newused == Err_overflow)
		{
		threshold *= 2;
		goto FINDHI;
		}
	else if (newused < Success)
		goto OUT;
	usedc += newused;

							/* Keep doing thresholds until we run out of
							 * colors. */
	hiwater = threshold;
	reduction_factor = 2;
	while (threshold >= 3)
		{
		if ((newused = find_newc(usedc, dcount-usedc, dest,
								 threshold, source, scount,curthresh))
			< Success)
			{
			if (newused != Err_overflow)
				goto OUT;
			else
				{
				newused = Success;
				threshold += threshold / 3;
				reduction_factor = 3;
				if (threshold >= hiwater)
					goto OUT;
				}
			}
		else
			{
			hiwater = threshold;
			threshold -= threshold/reduction_factor;
			}
		usedc += newused;
		}

OUT:

	if (curthresh != NULL)
		free(curthresh);

	return newused;
}
