/* blit8_.c */

#include <string.h>
#include "blit8_.h"
#include "clipit_.h"

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
