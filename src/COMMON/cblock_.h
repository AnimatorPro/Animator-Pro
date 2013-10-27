#ifndef CBLOCK_H
#define CBLOCK_H

#include "jimk.h"

/* Function: cdot
 *
 *  Draw a single pixel dot.  Clipped to 320x200.
 *
 *  dst - byte plane to draw on.
 *  x, y - screen position.
 *  col - dot colour.
 */
extern void cdot(UBYTE *dst, int x, int y, int col);

#endif
