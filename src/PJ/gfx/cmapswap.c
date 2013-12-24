#include "cmap.h"

void swap_cmaps(Cmap *a, Cmap *b)

/* swaps color palettes of a and b for minimum size of either */
{
USHORT csize;

	if(a->num_colors > b->num_colors)
		csize = b->num_colors;
	else 
		csize = a->num_colors;

	csize *= 3;
	swap_mem(a->ctab, b->ctab, csize);
}
