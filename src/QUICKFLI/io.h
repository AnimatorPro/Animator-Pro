#ifndef IO_H
#define IO_H

#include "jimk.h"

/* These are the scancodes for the escape key and spacebar. */
enum {
	ESC = 283,
	SPACE = 14624
};

/* Function: init_system */
extern int init_system(void);

/* Function: cleanup
 *
 *  Go back to old video mode and take out our clock interrupt handler.
 */
extern void cleanup(void);

/* Function: norm_pointer
 *
 * Add as much as possible of the offset of a pointer to the segment.
 */
extern void *norm_pointer(void *p);

/* Function: strobe_keys
 *
 *  Return 0 if no key, key scan code if there is a key.
 */
extern unsigned int strobe_keys(void);

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

/* Function: get80hz */
extern long get80hz(void);

#endif
