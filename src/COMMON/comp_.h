#ifndef COMP_H
#define COMP_H

#include "jimk.h"

/* Function: unrun */
extern void unrun(const UWORD *src, UWORD *dst);

/* Function: unlccomp
 * 
 *  Used to decompress all but the first frame.  It's a 'delta' scheme
 *  where pixels that are the same as the last frame are skipped over.
 *
 *  src - compressed source.  See FLI.DOC for details.
 *  dst - byte-plane to update.
 */
extern void unlccomp(const UBYTE *src, UBYTE *dst);

/* Function: fcuncomp
 *
 *  Decompress palette into a buffer in memory.
 *
 *  src - compressed colour source.  See FLI.DOC for details.
 *  dst - destination buffer.  256*3 bytes long.
 */
extern void fcuncomp(const UBYTE *src, UBYTE *dst);

#endif
