#ifndef UNBRUN1_H
#define UNBRUN1_H

#include "jimk.h"

/* Function: unbrun
 *
 *  Decompress 1st frame full size.
 *
 *  src - Compressed source.  See FLI.DOC for details.
 *  screen - Byte plane to decompress onto.
 *  linect - Number of lines of screen (always 320 now.)
 */
extern void unbrun(const UBYTE *src, UBYTE *dst, unsigned int linect);

#endif
