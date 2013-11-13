/* peekpok_.c */

#include <assert.h>
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

unsigned int
til_next_skip(const UBYTE *xs, const UBYTE *ys, unsigned int n,
		unsigned int mustmatch)
{
	unsigned int diffcount = 0;
	assert(mustmatch <= n);

	for (;;) {
		unsigned int num_same;
		unsigned int num_diff;

		num_diff = bcontrast(xs, ys, n);
		n -= num_diff;
		xs += num_diff;
		ys += num_diff;
		diffcount += num_diff;

		/* Check last couple of pixels. */
		if (n < mustmatch) {
			num_same = bcompare(xs, ys, n);

			if (num_same < n)
				diffcount += n;

			break;
		}

		num_same = bcompare(xs, ys, mustmatch);

		if (num_same == mustmatch)
			break;

		n -= num_same;
		xs += num_same;
		ys += num_same;
		diffcount += num_same;
	}

	return diffcount;
}

unsigned int
til_next_same(const UBYTE *src, unsigned int n, unsigned int mustmatch)
{
	unsigned int num_remaining = n;
	unsigned int num_examined = 0;

	while (num_remaining >= mustmatch) {
		unsigned int num_same = bsame(src, num_remaining);

		if (num_same >= mustmatch)
			return num_examined;

		src += num_same;
		num_examined += num_same;
		num_remaining -= num_same;
	}

	return n;
}
