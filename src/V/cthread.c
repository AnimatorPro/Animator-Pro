/* cthread.c - a rather clever little algorithm to rearrange your palette
   in such a way that it's likely to have a lot of smooth spectrums
   suitable for gradient fills and generally nicer to look at than
   your typical mess of a digitized palette. */

#include "jimk.h"
#include "flicmenu.h"

rthread_cmap(gotit, scmap, dcmap, inertia, ccount)
PLANEPTR gotit;  /* Pre-zeroed array of bytes ccount long used to
					keep track of which colors already have been
					put into destination cmap */
PLANEPTR scmap, dcmap;  /* Source and destination palettes */
int inertia;	/* Whether just pick closest color for next,
					or also have a tendency to continu in the same
					direction in color space we're going already */
int ccount;		/* # of colors in color map */
{
int i;
int lasti;
int closestix;
int closestval;
int temp;
int color;
UBYTE rgb[3];
UBYTE drgb[3];
int llasti;

llasti = lasti = 0;
for (color=0; color < ccount; color++)
	{
	copy_bytes(scmap+lasti*3, dcmap, 3);
	copy_bytes(dcmap, rgb, 3);
	if (inertia)
		{
		i = llasti*3;
		rgb[0] += (rgb[0] - scmap[i])/2;
		i++;
		rgb[1] += (rgb[1] - scmap[i])/2;
		i++;
		rgb[2] += (rgb[2] - scmap[i])/2;
		i++;
		}
	gotit[lasti] = 1;
	if (color == ccount-1)
		break;
	closestix = -1;
	closestval = 30000;	/* big number  - bigger than color_dif
							could return */
	for (i=0;i<ccount;i++)
		{
		if (!gotit[i])
			{
			if ((temp = color_dif(rgb, scmap+i*3)) < closestval)
				{
				closestval = temp;
				closestix = i;
				}
			}
		}
	llasti = lasti;
	lasti = closestix;
	dcmap += 3;
	}
}

