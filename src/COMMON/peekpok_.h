#ifndef PEEKPOK_H
#define PEEKPOK_H

#include "jimk.h"

/* Function: stuff_words */
extern void stuff_words(UWORD val, UWORD *dst, unsigned int n);

/* Function: copy_bytes */
extern void copy_bytes(const UBYTE *src, UBYTE *dst, unsigned int n);

/* Function: copy_structure */
extern void copy_structure(const void *src, void *dst, unsigned int n);

/* Function: exchange_words */
extern void exchange_words(UWORD *xs, UWORD *ys, unsigned int n);

/* Function: copy_words */
extern void copy_words(const UWORD *src, UWORD *dst, unsigned int n);

/* Function: bsame */
extern unsigned int bsame(const UBYTE *src, unsigned int n);

#define zero_structure(s, size) \
	stuff_words(0, s, ((unsigned int)(size))>>1)

#define zero_words(s, size) \
	stuff_words(0, s, (unsigned int)(size))

#define copy_cmap(s, d) \
	copy_structure(s, d, COLORS*3)

#endif
