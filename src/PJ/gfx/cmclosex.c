#include <assert.h>
#include "cmap.h"
#include "memory.h"


int closestc_excl(Rgb3 *rgb,  				/* color to match */
		 	      Rgb3 *ctab, int ccount,    /* color map to find it in */
		 	      UBYTE *ignore, int icount) /* what c's to ignore in ctab */
{
UBYTE checktab[256];      /* only good in byte a pixel world */
LONG closest_dif, dif;
SHORT dc;
register int i;
int closest = -1;

	clear_mem(checktab,ccount); /* all zeros */
	for(i =0;i<icount;++i)
		checktab[ignore[i]] = 1;

	closest_dif = 620000L;	/* arbitrary huge number */

	for (i=0; i<ccount; ++i)
	{
		if(!checktab[i])
		{
			dc = rgb->r-ctab->r;
			dif = dc*dc;
			dc = rgb->g-ctab->g;
			dif += dc*dc;
			dc = rgb->b-ctab->b;
			dif += dc*dc;
			if (dif < closest_dif)
			{
				closest_dif = dif;
				closest = i;
			}
		}
		++ctab;
	}

	assert(closest >= 0);
	return(closest);
}
