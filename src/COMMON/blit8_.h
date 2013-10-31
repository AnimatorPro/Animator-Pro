#ifndef BLIT8_H
#define BLIT8_H

#include "jimk.h"

/* Function: blit8
 *
 *  This blits from a byte-plane source to a byte-plane destination.
 *  It does not handle overlapping source and destination.  This is
 *  used all over the place: to save and restore what is under the
 *  cursor or under a menu, to paste the cel when key colour clear is
 *  turned off, etc.
 *
 *  width - width of blit in pixels.
 *  height - height of blit in pixels.
 *  sx, sy - coordinates of upper left corner of source.
 *  src - pointer to source byte-plane.
 *  sstride - how many bytes from one line of source to next.
 *  dx, dy - coordinates of upper left corner of destination.
 *  dst - pointer to destination byte-plane.
 *  dstride - how many bytes from one line of destination to next.
 */
extern void
blit8(int width, int height,
		int sx, int sy, const UBYTE *src, int sstride,
		int dx, int dy, UBYTE *dst, int dstride);

#endif
