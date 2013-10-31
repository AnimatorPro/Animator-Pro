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

#endif
