#ifndef IO_H
#define IO_H

#include "jimk.h"

/* These are the scancodes for the escape key and spacebar. */
enum {
	ESC = 283,
	SPACE = 14624
};

/* Function: cset_colors
 *
 *  Set the color palette hardware from a compressed source of format
 *  : WORD # of runs, run1, run2, ...,runn
 *  Each run is of form
 *  : BYTE colors to skip, BYTE colors to set, r1,g1,b1,r2,g2,b2,...,rn,gn,bn
 */
extern void cset_colors(const UBYTE *src);

/* Function: flip_video */
extern void flip_video(void);

/* Function: wait_vblank */
extern void wait_vblank(void);

/* Function: get80hz */
extern long get80hz(void);

#endif
