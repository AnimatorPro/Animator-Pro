/* mempeek.c */

#include "memory.h"

/* Function: pj_bsame */
unsigned int
pj_bsame(const void *src, unsigned int n)
{
	const uint8_t *x = src;
	unsigned int i;

	for (i = 0; i < n; i++) {
		if (x[i] != x[0])
			break;
	}

	return i;
}

/* Function: pj_bcompare */
unsigned int
pj_bcompare(const void *xs, const void *ys, unsigned int n)
{
	const uint8_t *x = xs;
	const uint8_t *y = ys;
	unsigned int i;

	for (i = 0; i < n; i++) {
		if (x[i] != y[i])
			break;
	}

	return i;
}

/* Function: pj_fcompare */
unsigned int
pj_fcompare(const void *xs, const void *ys, unsigned int n)
{
	const uint16_t *x = xs;
	const uint16_t *y = ys;
	unsigned int i;

	for (i = 0; i < n; i++) {
		if (x[i] != y[i])
			break;
	}

	return i;
}

/* Function: pj_dcompare */
unsigned int
pj_dcompare(const void *xs, const void *ys, unsigned int n)
{
	const uint32_t *x = xs;
	const uint32_t *y = ys;
	unsigned int i;

	for (i = 0; i < n; i++) {
		if (x[i] != y[i])
			break;
	}

	return i;
}

/* Function: pj_bcontrast */
unsigned int
pj_bcontrast(const void *xs, const void *ys, unsigned int n)
{
	const uint8_t *x = xs;
	const uint8_t *y = ys;
	unsigned int i;

	for (i = 0; i < n; i++) {
		if (x[i] == y[i])
			break;
	}

	return i;
}

/* Function: pj_til_next_skip */
unsigned int
pj_til_next_skip(const void *xs, const void *ys, unsigned int n,
		unsigned int mustmatch)
{
	const uint8_t *x = xs;
	const uint8_t *y = ys;
	unsigned int diffcount = 0;

	for (;;) {
		unsigned int num_same;
		unsigned int num_diff;

		num_diff = pj_bcontrast(x, y, n);
		n -= num_diff;
		x += num_diff;
		y += num_diff;
		diffcount += num_diff;

		/* Check last couple of pixels. */
		if (n < mustmatch) {
			num_same = pj_bcompare(x, y, n);

			if (num_same < n)
				diffcount += n;

			break;
		}

		num_same = pj_bcompare(x, y, mustmatch);

		if (num_same == mustmatch)
			break;

		n -= num_same;
		x += num_same;
		y += num_same;
		diffcount += num_same;
	}

	return diffcount;
}
