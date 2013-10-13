#include "jimk.h"

extern unsigned char alt_cmap[];

transit_to_white(cycle)
int cycle;  /* the number of cycles over which to make the transition */
/* take the vf.cmp and graduate to white (= 63,63,63) */
{
UBYTE *d;
int i;

copy_cmap(vf.cmap,alt_cmap);

cycle++;
while (cycle-- > 0)
	{
	d=alt_cmap;
	for (i=0; i < COLORS * 3; i++)
		{
		*d++ = *d + (63 - *d )/cycle; 	
		}
	wait_sync();
	jset_colors(0, COLORS, alt_cmap);
	}
copy_cmap(alt_cmap,vf.cmap);
}


