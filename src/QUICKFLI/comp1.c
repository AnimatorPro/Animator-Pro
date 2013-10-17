/* comp1.c */

#include <string.h>
#include "comp1.h"

void
fcuncomp(const UBYTE *src, UBYTE *dst)
{
	unsigned int count = ((const UWORD *)src)[0];
	src += 2;

	for (; count > 0; count--) {
		int nskip = src[0];
		int ncopy = src[1];

		if (ncopy == 0)
			ncopy = 256;

		src += 2;
		dst += 3 * nskip;

		memcpy(dst, src, 3 * ncopy);

		src += 3 * ncopy;
		dst += 3 * ncopy;
	}
}
