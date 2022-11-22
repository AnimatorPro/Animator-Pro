#ifndef ROTATE_H
#define ROTATE_H

#include "jimk.h"

/* Function: diag_to_table
 *
 *  Copy from a diagonal line in s to a horizontal line dsize long starting
 *  at 'dtable'.
 */
extern void
diag_to_table(PLANEPTR s, int sbpr, PLANEPTR dtable, int dsize,
		int x0, int y0, int x1, int y1);

#endif
