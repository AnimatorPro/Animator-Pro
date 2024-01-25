#ifndef LCCOMP_H
#define LCCOMP_H

#include "jimk.h"

/* Function: lccomp */
extern UWORD *
lccomp(const char *s1, const char *s2, UWORD *cbuf,
		unsigned int width, unsigned int height);

/* Function: brun */
extern UWORD *
brun(const char *s1, const char *s2, WORD *cbuf, int width, int height);

#endif
