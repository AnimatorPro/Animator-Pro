#ifndef CBLOCK_H
#define CBLOCK_H

#include "jimk.h"

/* Function: xorblock
 *
 *  Exclusive or a rectangle with colour.  Clipped to fit 320x200 screen.
 *  Used for text cursor.
 *
 *  dst - byte plane to draw on.
 *  x,y - upper left corner of rectangle.
 *  width, height - dimensions of rectangle in pixels.
 *  col - colour to xor with block.
 */
extern void xorblock(UBYTE *dst, int x, int y, int width, int height, int col);

/* Function: cblock
 *
 *  Draw a rectangle in a solid colour.  Clipped to fit 320x200 screen.
 *  Used heavily by menu drawing routines.
 *
 *  dst - byte plane to draw on.
 *  x, y - upper left corner of rectangle.
 *  width, height - dimensions of rectangle in pixels.
 *  col - colour to set block.
 */
extern void cblock(UBYTE *dst, int x, int y, int width, int height, int col);

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

/* Function: cvli
 *
 *  Draw a vertical line in a solid colour.  Not clipped.  Used by
 *  menu routines.
 *
 *  dst - byte plane to draw on.
 *  x, y - left end of line.
 *  height - height of line.
 *  col - colour of line.
 */
extern void cvli(UBYTE *dst, int x, int y, int height, int col);

/* Function: getd
 *
 *  Return colour of pixel.  Not clipped.
 *
 *  src - byte plane to read from.
 *  x, y - screen position.
 */
extern UBYTE getd(const UBYTE *src, int x, int y);

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

#define xorrop(col, x, y, width, height) \
	xorblock(vf.p, x, y, width, height, col)

#define colblock(col, x, y, x2, y2)  \
	cblock(vf.p, x, y, (x2)-(x)+1, (y2)-(y)+1, col)

#define colrop(col, x, y, width, height) \
	cblock(vf.p, x, y, (width+1), (height+1), col)

#define color_hslice(y, height, col) \
	cblock(vf.p, 0, y, vf.w, height, col)

#define hline(y, x0, x1, col) \
	chli(vf.p, x0, y, (x1)-(x0)+1, col)

#define vline(x, y0, y1, col) \
	cvli(vf.p, x, y0, (y1)-(y0)+1, col)

#define getdot(x, y) \
	getd(vf.p, x, y)

#define putdot(x, y, col) \
	cdot(vf.p, x, y, col)

#endif /* SLUFF */

#endif
