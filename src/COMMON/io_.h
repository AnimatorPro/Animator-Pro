#ifndef IO_H
#define IO_H

#include "jimk.h"

#define CTRL  0x0004

enum {
	CTRL_D      = 4,
	CTRL_F      = 6,
	CTRL_W      = 23,
	ESC         = 283,
	BACKSPACE   = 3592,
	SPACE       = 14624,
	HOMEKEY     = 0x4700,
	UARROW      = 0x4800,
	PAGEUP      = 0x4900,
	LARROW      = 0x4b00,
	RARROW      = 0x4d00,
	ENDKEY      = 0x4f00,
	DARROW      = 0x5000,
	PAGEDN      = 0x5100,
	DELKEY      = 0x5300
};

/* Function: cset_colors
 *
 *  Set the color palette hardware from a compressed source of format
 *  : WORD # of runs, run1, run2, ...,runn
 *  Each run is of form
 *  : BYTE colors to skip, BYTE colors to set, r1,g1,b1,r2,g2,b2,...,rn,gn,bn
 */
extern void cset_colors(const UBYTE *src);

/* Function: jset_colors */
extern void jset_colors(int start, int length, const UBYTE *cmap);

/* Function: flip_video */
extern void flip_video(void);

/* Function: wait_vblank */
extern void wait_vblank(void);

/* Function: wait_novblank */
extern void wait_novblank(void);

/* Function: get80hz */
extern long get80hz(void);

#endif
