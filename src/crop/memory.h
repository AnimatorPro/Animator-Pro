#ifndef MEMORY_H
#define MEMORY_H

#if defined(__TURBOC__)
#include "jimk.h"
extern void freemem(UWORD *pt);
extern void *laskmem(long size);
extern void *askmem(unsigned int size);
extern void *lbegmem(long size);
extern void *begmem(unsigned int size);
extern void gentle_freemem(UWORD *pt);
extern int init_mem(void);
#else
#include <stdlib.h>
#define freemem(pt)         free(pt)
#define laskmem(size)       malloc(size)
#define  askmem(size)       malloc(size)
#define lbegmem(size)       malloc(size)
#define  begmem(size)       malloc(size)
#define gentle_freemem(pt)  free(pt)
#endif

extern void outta_memory(void);

#endif
