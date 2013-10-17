#ifndef COMP1_H
#define COMP1_H

#include "jimk.h"

/* Function: unlccomp
 * 
 *  Used to decompress all but the first frame.  It's a 'delta' scheme
 *  where pixels that are the same as the last frame are skipped over.
 *
 *  src - Compressed source.  See FLI.DOC for details.
 *  dst - Byte-plane to update.
 */
extern void unlccomp(const UBYTE *src, UBYTE *dst);

/* Function: fcuncomp
 *
 *  Decompress palette into a buffer in memory.
 *
 *  src - Compressed color source.  See FLI.DOC for details.
 *  dst - Destination buffer.  256*3 bytes long.
 */
extern void fcuncomp(const UBYTE *src, UBYTE *dst);

#endif
