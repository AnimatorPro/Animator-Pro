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
copy_bytes(const UBYTE *src, UBYTE *dst, unsigned int n)
{
	memmove(dst, src, n);
}

void
copy_structure(const void *src, void *dst, unsigned int n)
{
	copy_words(src, dst, n / 2);
}

void
exchange_words(UWORD *xs, UWORD *ys, unsigned int n)
{
	for (; n > 0; n--) {
		UWORD x = *xs;
		UWORD y = *ys;

		*xs++ = y;
		*ys++ = x;
	}
}

void
copy_words(const UWORD *src, UWORD *dst, unsigned int n)
{
	memmove(dst, src, 2 * n);
}

unsigned int
bsame(const UBYTE *src, unsigned int n)
{
	unsigned int i;

	for (i = 0; i < n; i++) {
		if (src[i] != src[0])
			break;
	}

	return i;
}

unsigned int
fsame(const UWORD *src, unsigned int n)
{
	unsigned int i;

	for (i = 0; i < n; i++) {
		if (src[i] != src[0])
			break;
	}

	return i;
}

unsigned int
bcontrast(const UBYTE *xs, const UBYTE *ys, unsigned int n)
{
	unsigned int i;

	for (i = 0; i < n; i++) {
		if (xs[i] == ys[i])
			break;
	}

	return i;
}

unsigned int
bcompare(const UBYTE *xs, const UBYTE *ys, unsigned int n)
{
	unsigned int i;

	for (i = 0; i < n; i++) {
		if (xs[i] != ys[i])
			break;
	}

	return i;
}

unsigned int
fcompare(const UWORD *xs, const UWORD *ys, unsigned int n)
{
	unsigned int i;

	for (i = 0; i < n; i++) {
		if (xs[i] != ys[i])
			break;
	}

	return i;
}
