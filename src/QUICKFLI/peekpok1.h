#ifndef PEEKPOK1_H
#define PEEKPOK1_H

#include "jimk.h"

/* Function: stuff_words */
extern void stuff_words(UWORD val, UWORD *dst, unsigned int n);

/* Function: copy_words */
extern void copy_words(const UWORD *src, UWORD *dst, unsigned int n);

#endif
