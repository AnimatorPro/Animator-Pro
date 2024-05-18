#include "memory.h"

/* Function: pj_enorm_pointer
 *
 *  Forces a pointer to even alignment on 386.  On 8086 would also
 *  fold in as much as possible of the offset into the segment.
 */
void *
pj_enorm_pointer(void *p)
{
	const intptr_t x = (intptr_t)p;
	return (void *) ((x + 1) & ~0x1);
}
