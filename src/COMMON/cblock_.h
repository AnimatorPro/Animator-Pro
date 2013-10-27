#ifndef CBLOCK_H
#define CBLOCK_H

#include "jimk.h"

/* Function: chli
 *
 *  Draw a horizontal line in a solid colour.  Not clipped.  Used by
 *  menu routines.
 *
 *  dst - byte plane to draw on.
 *  x, y - left end of line.
 *  width - width of line.
 *  col - colour of line.
 */
extern void chli(UBYTE *dst, int x, int y, int width, int col);

/* Function: cdot
 *
 *  Draw a single pixel dot.  Clipped to 320x200.
 *
 *  dst - byte plane to draw on.
 *  x, y - screen position.
 *  col - dot colour.
 */
extern void cdot(UBYTE *dst, int x, int y, int col);

#ifndef SLUFF

#define hline(y, x0, x1, col) \
	chli(vf.p, x0, y, (x1)-(x0)+1, col)

#endif /* SLUFF */

#endif
