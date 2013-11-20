#ifndef LCCOMP_H
#define LCCOMP_H

#include "jimk.h"

/* Function: lccomp */
extern unsigned int *
lccomp(const char *s1, const char *s2, unsigned int *cbuf,
		unsigned int width, unsigned int height);

/* Function: brun */
extern unsigned int *
brun(const char *s1, const char *s2, int *cbuf, int width, int height);

#endif
