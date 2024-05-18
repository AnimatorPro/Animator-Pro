/* cmapshif.c */

#include "cmap.h"

/* function: pj_shift_cmap */
void
pj_shift_cmap(const UBYTE *src, UBYTE *dst, unsigned int n)
{
	unsigned int i;

	for (i = 0; i < n; i++)
		dst[i] = src[i] * 4;
}
