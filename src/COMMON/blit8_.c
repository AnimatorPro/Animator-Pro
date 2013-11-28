/* blit8_.c */

#include <string.h>
#include "blit8_.h"
#include "clipit_.h"

void
a1blit(int width, int height,
		int sx, int sy, const UBYTE *src, int sstride,
		int dx, int dy, UBYTE *dst, int dstride, int col)
{
	if (!clipblit_(&width, &height, &sx, &sy, &dx, &dy))
		return;

	src += sstride * sy + (sx >> 3);
	dst += dstride * dy + dx;

	for (; height > 0; height--) {
		const UBYTE *sp = src;
		UBYTE *dp = dst;
		UBYTE bits = *sp++;
		UBYTE mask = 0x80 >> (sx & 7);
		int count = width;

		for (;;) {
			if (bits & mask) {
				*dp = col;
			}

			if (--count <= 0)
				break;

			dp++;
			mask >>= 1;
			if (mask == 0) {
				bits = *sp++;
				mask = 0x80;
			}
		}

		src += sstride;
		dst += dstride;
	}
}

void
a2blit(int width, int height,
		int sx, int sy, const UBYTE *src, int sstride,
		int dx, int dy, UBYTE *dst, int dstride, int fg, int bg)
{
	if (!clipblit_(&width, &height, &sx, &sy, &dx, &dy))
		return;

	src += sstride * sy + (sx >> 3);
	dst += dstride * dy + dx;

	for (; height > 0; height--) {
		const UBYTE *sp = src;
		UBYTE *dp = dst;
		UBYTE bits = *sp++;
		UBYTE mask = 0x80 >> (sx & 7);
		int count = width;

		for (;;) {
			if (bits & mask) {
				*dp = fg;
			}
			else {
				*dp = bg;
			}

			if (--count <= 0)
				break;

			dp++;
			mask >>= 1;
			if (mask == 0) {
				bits = *sp++;
				mask = 0x80;
			}
		}

		src += sstride;
		dst += dstride;
	}
}

void
blit8(int width, int height,
		int sx, int sy, const UBYTE *src, int sstride,
		int dx, int dy, UBYTE *dst, int dstride)
{
	if (!clipblit_(&width, &height, &sx, &sy, &dx, &dy))
		return;

	src += sstride * sy + sx;
	dst += dstride * dy + dx;

	for (; height > 0; height--) {
		memcpy(dst, src, width);

		src += sstride;
		dst += dstride;
	}
}
