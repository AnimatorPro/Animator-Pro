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

void
tblit8(int width, int height,
		int sx, int sy, const UBYTE *src, int sstride,
		int dx, int dy, UBYTE *dst, int dstride, int transcol)
{
	if (!clipblit_(&width, &height, &sx, &sy, &dx, &dy))
		return;

	src += sstride * sy + sx;
	dst += dstride * dy + dx;

	for (; height > 0; height--) {
		int x;

		for (x = 0; x < width; x++) {
			if (src[x] != transcol)
				dst[x] = src[x];
		}

		src += sstride;
		dst += dstride;
	}
}

void
tmove8(int width, int height,
		int sx, int sy, const UBYTE *src, int sstride,
		int dx, int dy, UBYTE *dst, int dstride, int transcol,
		const UBYTE *undo)
{
	if (!clipblit_(&width, &height, &sx, &sy, &dx, &dy))
		return;

	src += sstride * sy + sx;
	dst += dstride * dy + dx;
	undo += dstride * dy + dx;

	for (; height > 0; height--) {
		int x;

		for (x = 0; x < width; x++)
			dst[x] = (src[x] == transcol) ? undo[x] : src[x];

		src += sstride;
		dst += dstride;
		undo += dstride;
	}
}

void
ublit8(int width, int height,
		int sx, int sy, const UBYTE *src, int sstride,
		int dx, int dy, UBYTE *dst, int dstride, int transcol)
{
	if (!clipblit_(&width, &height, &sx, &sy, &dx, &dy))
		return;

	src += sstride * sy + sx;
	dst += dstride * dy + dx;

	for (; height > 0; height--) {
		int x;

		for (x = 0; x < width; x++) {
			if (dst[x] == transcol)
				dst[x] = src[x];
		}

		src += sstride;
		dst += dstride;
	}
}

void
shrink5(int width, int height,
		int sx, int sy, const UBYTE *src, int sstride,
		int dx, int dy, UBYTE *dst, int dstride)
{
	src += sstride * sy + sx;
	dst += dstride * dy + dx;

	for (; height > 0; height--) {
		int x;

		for (x = 0; x < width; x++)
			dst[x] = src[5 * x];

		src += sstride;
		dst += dstride;
	}
}

static void
hafline2(int swidth, const UBYTE *src, UBYTE *dst)
{
	int x;

	for (x = 0; x < swidth; x++) {
		dst[2 * x + 0] = src[x];
		dst[2 * x + 1] = src[x];
	}
}

void
zoomblit(int swidth, int dheight,
		int sx, int sy, const UBYTE *src, int sstride,
		int dx, int dy, UBYTE *dst, int dstride)
{
	src += sstride * sy + sx;
	dst += dstride * dy + dx;

	if (dy & 1) {
		hafline2(swidth, src, dst);
		src += sstride;
		dst += dstride;
		dheight--;
	}

	for (dheight = dheight / 2; dheight > 0; dheight--) {
		hafline2(swidth, src, dst);
		dst += dstride;

		hafline2(swidth, src, dst);
		dst += dstride;

		src += sstride;
	}
}

static void
hafline4(int swidth, const UBYTE *src, UBYTE *dst)
{
	int x;

	for (x = 0; x < swidth; x++) {
		dst[4 * x + 0] = src[x];
		dst[4 * x + 1] = src[x];
		dst[4 * x + 2] = src[x];
		dst[4 * x + 3] = src[x];
	}
}

void
zoom4blit(int swidth, int dheight,
		int sx, int sy, const UBYTE *src, int sstride,
		int dx, int dy, UBYTE *dst, int dstride)
{
	src += sstride * sy + sx;
	dst += dstride * dy + dx;

	if (dy & 3) {
		int y;

		for (y = 4 - (dy & 3); y < 4; y++) {
			hafline4(swidth, src, dst);
			dst += dstride;
			dheight--;
		}

		src += sstride;
	}

	for (dheight = dheight / 4; dheight > 0; dheight--) {
		hafline4(swidth, src, dst);
		dst += dstride;

		hafline4(swidth, src, dst);
		dst += dstride;

		hafline4(swidth, src, dst);
		dst += dstride;

		hafline4(swidth, src, dst);
		dst += dstride;

		src += sstride;
	}
}
