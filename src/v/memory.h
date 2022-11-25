#ifndef MEMORY_H
#define MEMORY_H

#if defined(__TURBOC__)
/* #define USE_MEMORY_MANAGEMENT */
#endif

#ifdef USE_MEMORY_MANAGEMENT

#include "jimk.h"

#define freemem(pt) freememory((UWORD *)(pt))
extern void *laskmem(long size);
extern void *askmem(unsigned int size);
extern void *lbegmem(long size);
extern void *begmem(unsigned int size);
extern void *begmemc(unsigned int size);
extern void gentle_freemem(void *pt);
extern void freememory(UWORD *pt);

#else /* USE_MEMORY_MANAGEMENT */

#include <stdlib.h>

#if defined(__TURBOC__)
#define laskmem(size)       farmalloc((size) > 0 ? (size) : 1)
#define lbegmem(size)       farmalloc(size)
#else /* __TURBOC__ */
#define laskmem(size)       malloc((size) > 0 ? (size) : 1)
#define lbegmem(size)       malloc(size)
#endif /* __TURBOC__ */

#define freemem(pt)         free(pt)
#define  askmem(size)       malloc(size)
#define  begmem(size)       malloc(size)
#define  begmemc(size)      calloc((size) / 2, 2)
#define gentle_freemem(pt)  free(pt)

#endif /* USE_MEMORY_MANAGEMENT */

extern void outta_memory(void);

#endif
