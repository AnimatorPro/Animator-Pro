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
void pj_gentle_free(void *pt);

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

extern int nonzero_bytes(const UBYTE *c, int array_size);
extern unsigned int pj_bsame(const void *src, unsigned int n);
extern unsigned int pj_bcompare(const void *xs, const void *ys, unsigned int n);
extern unsigned int pj_fcompare(const void *xs, const void *ys, unsigned int n);
extern unsigned int pj_dcompare(const void *xs, const void *ys, unsigned int n);

extern unsigned int
pj_bcontrast(const void *xs, const void *ys, unsigned int n);

extern unsigned int
pj_til_next_skip(const void *xs, const void *ys, unsigned int n,
		unsigned int mustmatch);

extern unsigned int
pj_til_next_same(const void *src, unsigned int n, unsigned int mustmatch);

extern void zero_lots(void *pt, LONG size);
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

/* memtrd.c */
extern void *trd_askmem(unsigned int count);
extern void *trd_laskmem(long count);
extern void *trd_askcmem(unsigned int count);
extern long trd_freemem(void *pt);
extern void trd_freez(void **pt);
extern void *trd_flush_alloc(long size);

/* memtrdlo.c */
extern Errcode init_mem(long max_mem);
extern long largest_frag(void);
extern void *lo_askmem(long size);
extern long lo_freemem(void *pt);

/* freem.c */
extern Errcode fake_push(void);
extern void free_buffers(void);
extern Errcode pop_screen_id(LONG check_id);
extern void fake_push_cel(void);
extern void fake_pop_cel(void);
extern Errcode push_cel(void);
extern Errcode pop_cel(void);
extern Errcode push_most(void);
extern Errcode push_pics_id(LONG time_id);
extern void set_trd_maxmem(void);
extern void rem_check_tflx_toram(void);
extern void add_check_tflx_toram(void);
extern void pop_most(void);
extern void maybe_push_most(void);
extern void maybe_pop_most(void);
extern void push_inks(void);
extern void ink_push_cel(void);
extern void ink_pop_cel(void);
extern void pop_inks(void);

#endif
