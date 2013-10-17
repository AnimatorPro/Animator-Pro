#ifndef COMP1_H
#define COMP1_H

#include "jimk.h"

/* Function: fcuncomp
 *
 *  Decompress palette into a buffer in memory.
 *
 *  src - Compressed color source.  See FLI.DOC for details.
 *  dst - Destination buffer.  256*3 bytes long.
 */
extern void fcuncomp(const UBYTE *src, UBYTE *dst);

#endif
