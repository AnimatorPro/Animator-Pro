
/* undone */
/* cthread.c - a rather clever little algorithm to rearrange your palette
   in such a way that it's likely to have a lot of smooth spectrums
   suitable for gradient fills and generally nicer to look at than
   your typical mess of a digitized palette. */

#include "memory.h"
#include "cmap.h"

static UBYTE adddif(UBYTE orig, UBYTE next)
/* return orig plus half of difference between orig and next.  */
{
int temp;

temp = orig;
temp += (temp - next)/2;
if (temp < 0)
	temp = 0;
return(temp);
}

void
rthread_cmap(UBYTE *gotit, /* Pre-zeroed array of bytes ccount long used to
					            keep track of which colors already have been
								put into destination cmap */
			 Rgb3 *scmap, 
			 Rgb3 *dcmap, /* Source and destination palettes */
			 int inertia,	 /* Whether just pick closest color for next,
								or also have a tendency to continu in the same
								direction in color space we're going already */
			 int ccount)	 /* # of colors in color map */
{
int i;
int lasti;
int closestix;
int closestval;
int temp;
int color;
Rgb3 rgb;
int llasti;

	llasti = lasti = 0;
	for (color=0; color < ccount; color++)
	{
		rgb = *dcmap = scmap[lasti];
		if (inertia)
		{
			rgb.r = adddif(rgb.r, scmap[llasti].r);
			rgb.g = adddif(rgb.g, scmap[llasti].g);
			rgb.b = adddif(rgb.b, scmap[llasti].b);
		}
		gotit[lasti] = 1;
		if (color == ccount-1)
			break;
		closestix = -1;
		closestval = ((unsigned)~0)>>1; /* big number  - bigger than color_dif
									 could return */

		for (i=0;i<ccount;i++)
		{
			if (!gotit[i])
			{
				if ((temp = color_dif(&rgb, scmap+i)) < closestval)
				{
					closestval = temp;
					closestix = i;
				}
			}
		}
		llasti = lasti;
		lasti = closestix;
		++dcmap;
	}
}

