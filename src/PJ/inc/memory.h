#ifndef MEMORY_H
#define MEMORY_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

/* stack check cookies */
#define STACK_COOKIES 0xCC
void pj_init_stack();
int pj_get_stack_used();

extern void *pj_enorm_pointer(void *);	/* WORD-alligns pointer */
extern void *begmem(unsigned);
extern void *lbegmem(long);
extern void *pj_malloc(unsigned);
extern void *pj_zalloc(unsigned);
	/* return pointer to cleared buffer of size long, or NULL */
Errcode ealloc(void **pt, long size);
	/* allocate cleared buffer of size into *pt.
	 * Return Err_no_memory or Success */

void pj_free(void *p);
void pj_freez(void *pmem);	/* actually a pointer to the memory pointer */

/* t is a pointer to a structure.  news(t) points t to a freshly
   allocated zeroed block of memory and returns an error code */
#define news(t)  ((((t) = pj_zalloc(sizeof(*(t)))) != NULL) \
	? Success : Err_no_memory)

#define zero_structure(s, size) pj_stuff_words(0, s, ((unsigned)(size))>>1)

/* this is to be used instead of memset() ansi function */
#define clear_mem(mem,size) pj_stuff_bytes(0,mem,size)
#define clear_struct(s) clear_mem((s),sizeof(*(s)))

void swap_mem(void *srca, void *srcb, int count);

/* macro make s->d order of args */
#define copy_mem(src,dst,bytesize) memcpy((dst),(src),(bytesize))

void back_copy_mem(void *s,void *d,int count);
ULONG mem_crcsum(void *sbuf, LONG size);

/* stuff NULL pointers in source with contents of dest of same index */
void pj_load_array_nulls(void **source, void **dest, int num_pointers);

extern unsigned int pj_bsame(const void *src, unsigned int n);
extern unsigned int pj_bcompare(const void *xs, const void *ys, unsigned int n);
extern unsigned int pj_fcompare(const void *xs, const void *ys, unsigned int n);
extern unsigned int pj_dcompare(const void *xs, const void *ys, unsigned int n);

extern unsigned int
pj_bcontrast(const void *xs, const void *ys, unsigned int n);

extern void pj_stuff_bytes(uint8_t data, void *dst, unsigned int n);
extern void pj_stuff_words(uint16_t data, void *dst, unsigned int n);
extern void pj_stuff_dwords(uint32_t data, void *dst, unsigned int n);
extern void pj_stuff_pointers(void *data, void *dst, unsigned int n);

extern void pj_copy_bytes(const void *src, void *dst, unsigned int n);
extern void pj_copy_words(const void *src, void *dst, unsigned int n);
extern void pj_copy_structure(const void *src, void *dst, unsigned int n);

extern void pj_xor_bytes(uint8_t data, void *dst, unsigned int n);

extern void pj_xlate(const uint8_t *table, uint8_t *xs, unsigned int n);

extern long mem_free, init_mem_free;
extern long pj_mem_used, pj_max_mem_used;

#endif /* MEMORY_H */
