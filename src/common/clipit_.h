#ifndef CLIPIT_H
#define CLIPIT_H

/* Function: clipblit_
 *
 *  Raster blit oriented clip.  Returns 0 if clipped out,
 *  otherwise adjusts width, height, sx, sy, dx, dy
 *  to clipped values.
 */
extern int
clipblit_(int *rwidth, int *rheight, int *rsx, int *rsy, int *rdx, int *rdy);

/* Function: clipblit2 */
extern int
clipblit2(int *rwidth, int *rheight,
		int *rsx, int *rsy, int swidth, int sheight,
		int *rdx, int *rdy, int dwidth, int dheight);

#endif
