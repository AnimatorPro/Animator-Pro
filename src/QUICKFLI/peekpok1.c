/* peekpok1.c */

#include <string.h>
#include "peekpok1.h"

void
copy_words(const UWORD *src, UWORD *dst, unsigned int n)
{
	memcpy(dst, src, 2 * n);
}
