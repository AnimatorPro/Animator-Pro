#ifndef UNBRUN_H
#define UNBRUN_H

#include "jimk.h"

/* Function: unbrun
 *
 *  Decompress 1st frame full size.
 *
 *  src - compressed source.  See FLI.DOC for details.
 *  dst - byte plane to decompress onto.
 *  linect - number of lines of screen (always 320 now.)
 */
extern void unbrun(const UBYTE *src, UBYTE *dst, unsigned int linect);

#endif
