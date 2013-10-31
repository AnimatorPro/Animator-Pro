#ifndef A2BLIT_H
#define A2BLIT_H

#include "jimk.h"

/* Function: a2blit
 *
 *  Blit from single bit-plane source to byte-a-pixel destination.
 *  1's in source are set to foreground colour in destination.  0's in source 
 *  are set to background colour in destination.  Used to implement text
 *  in string requestors and in places where one message writes over another.
 *
 *  width - width of blit in pixels.
 *  height - height of blit in pixels.
 *  sx, sy - coordinates of upper left corner of source.
 *  src - pointer to source bit-plane.
 *  sstride - how many bytes from one line of source to next.
 *  dx, dy - coordinates of upper left corner of destination.
 *  dst - pointer to destination byte-plane.
 *  dstride - how many bytes from one line of destination to next.
 *  fg - colour 1's in source are set to in destination.
 *  bg - colour 0's in source are set to in destination.
 */
extern void
a2blit(int width, int height,
		int sx, int sy, const UBYTE *src, int sstride,
		int dx, int dy, UBYTE *dst, int dstride, int fg, int bg);

#endif
