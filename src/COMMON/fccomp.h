#ifndef FCCOMP_H
#define FCCOMP_H

#include "jimk.h"

/* Function: fccomp
 *
 *  Compress an rgb triples color map just doing 'skip' compression.
 */
extern UWORD *
fccomp(const char *s1, const char *s2, UWORD *cbuf, unsigned int count);

#endif
