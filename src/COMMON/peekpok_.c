/* peekpok_.c */

#include <string.h>
#include "peekpok_.h"

void
stuff_words(UWORD val, UWORD *dst, unsigned int n)
{
	unsigned int i;

	for (i = 0; i < n; i++)
		dst[i] = val;
}

void
copy_words(const UWORD *src, UWORD *dst, unsigned int n)
{
	memcpy(dst, src, 2 * n);
}
