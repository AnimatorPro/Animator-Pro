#ifndef A1BLIT_H
#define A1BLIT_H

#include "jimk.h"

/* Function: a1blit
 *
 *  Blit from single bit-plane source to byte-a-pixel destination.
 *  1's in source are set to colour in destination.  0's in source
 *  have no effect on destination.  Used to implement menu icons,
 *  most text, and the cursor.
 *
 *  width - width of blit in pixels.
 *  height - height of blit in pixels.
 *  sx, sy - coordinates of upper left corner of source.
 *  src - pointer to source bit-plane.
 *  sstride - how many bytes from one line of source to next.
 *  dx, dy - coordinates of upper left corner of destination.
 *  dst - pointer to destination byte-plane.
 *  dstride - how many bytes from one line of destination to next.
 *  col - colour 1's in source are set to in destination.
 */
extern void
a1blit(int width, int height,
		int sx, int sy, const UBYTE *src, int sstride,
		int dx, int dy, UBYTE *dst, int dstride, int col);

#ifndef SLUFF

#define cdraw_brush(brush, x, y, col) \
	a1blit(16, 16, 0, 0, brush, 2, (x)-8, (y)-8, vf.p, vf.bpr, col)

#define draw_brush(brush, x, y, col) \
	a1blit(16, 16, 0, 0, brush, 2, x, y, vf.p, vf.bpr, col)

#endif /* SLUFF */

#endif
