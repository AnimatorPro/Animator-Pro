/* a1blit_.c */

#include "a1blit_.h"
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
