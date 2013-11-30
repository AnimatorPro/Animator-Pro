#ifndef TRUECOL_H
#define TRUECOL_H

#include "jimk.h"

/* Function: closestc
 *
 *  Find the closest colour in the colour map to some given
 *  RGB value.  Assumes RGB are each byte values.  May even break if
 *  they grow larger than 63.  (This is what VGA uses.)
 */
extern unsigned int closestc(const UBYTE *rgb, const UBYTE *cmap, int count);

/* Function: colorave */
extern void
colorave(int x, int y, UBYTE *rgb, const UBYTE *screen, const UBYTE *cmap);

#endif
