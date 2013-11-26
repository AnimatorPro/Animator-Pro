#ifndef PEEKPOK_H
#define PEEKPOK_H

#include "jimk.h"

/* Function: stuff_bytes */
extern void stuff_bytes(UBYTE val, UBYTE *dst, unsigned int n);

/* Function: stuff_words */
extern void stuff_words(UWORD val, UWORD *dst, unsigned int n);

/* Function: xor_words */
extern void xor_words(UWORD val, UWORD *dst, unsigned int n_8);

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

/* Function: fsame */
extern unsigned int fsame(const UWORD *src, unsigned int n);

/* Function: bcontrast */
extern unsigned int bcontrast(const UBYTE *xs, const UBYTE *ys, unsigned int n);

/* Function: bcompare */
extern unsigned int bcompare(const UBYTE *xs, const UBYTE *ys, unsigned int n);

/* Function: fcompare */
extern unsigned int fcompare(const UWORD *xs, const UWORD *ys, unsigned int n);

/* Function: til_next_skip */
extern unsigned int
til_next_skip(const UBYTE *xs, const UBYTE *ys, unsigned int n,
		unsigned int mustmatch);

/* Function: til_next_same */
extern unsigned int
til_next_same(const UBYTE *src, unsigned int n, unsigned int mustmatch);

#define zero_structure(s, size) \
	stuff_words(0, s, ((unsigned int)(size))>>1)

#define zero_words(s, size) \
	stuff_words(0, s, (unsigned int)(size))

#define copy_cmap(s, d) \
	copy_structure(s, d, COLORS*3)

#endif
