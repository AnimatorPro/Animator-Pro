#ifndef PTR_H
#define PTR_H

#if defined(__TURBOC__)

extern void *make_ptr();
extern void *norm_pointer(void *p);
extern unsigned int ptr_offset();
extern unsigned int ptr_seg();
extern void *ptr_next_seg(void *p);
extern long pt_to_long();
extern void *long_to_pt();

#else /* __TURBOC__ */

#include <stdint.h>
#define make_ptr(x)     (x)
#define ptr_offset(x)
#define ptr_seg(x)
#define norm_pointer(x) (x)
#define ptr_next_seg(x) (x)
#define pt_to_long(x)   ((intptr_t)(x))
#define long_to_pt(x)   ((void *)(x))

#endif /* __TURBOC__ */

extern void *enorm_pointer(void *p);

#endif
