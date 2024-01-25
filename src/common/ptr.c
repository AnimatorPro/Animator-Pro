/* ptr.c */

#include "jimk.h"
#include "ptr.h"

#ifndef make_ptr
/* fool C into thinking a long is a pointer */
void *
make_ptr(pt)
void *pt;
{
	return pt;
}
#endif /* make_ptr */

#if !defined(__TURBOC__)
void *
enorm_pointer(void *p)
{
	intptr_t x = (intptr_t)p;
	return (void *) ((x + 1) & ~0x1);
}
#endif /* __TURBOC__ */

#ifndef ptr_offset
unsigned int
ptr_offset(offset, seg)
int offset, seg;
{
	return offset;
}
#endif /* ptr_offset */

#ifndef ptr_seg
unsigned int
ptr_seg(offset, seg)
int offset, seg;
{
	return seg;
}
#endif /* ptr_seg */

#ifndef ptr_next_seg
void *
ptr_next_seg(void *p)
{
	return make_ptr(0, ptr_seg(p) + 1);
}
#endif /* ptr_next_seg */

#ifndef pt_to_long
long 
pt_to_long(offset, seg)
unsigned offset, seg;
{
	long result;

	result = seg;
	result <<= 4;
	result += offset;
	return result;
}
#endif /* pt_to_long */

#ifndef long_to_pt
void *
long_to_pt(l)
unsigned long l;
{
	unsigned segment, offset;

	offset = (l&15);
	l >>= 4;
	segment = l;
	return make_ptr(offset, segment);
}
#endif /* long_to_pt */
