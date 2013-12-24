#include "stdtypes.h"
#include "ptrmacro.h"

void back_copy_mem(void *src,void *dst,int count)
{
register UBYTE *s;
register UBYTE *d;

	s = OPTR(src,count);
	d = OPTR(dst,count);
	while(d > (UBYTE *)dst)
		*(--d) = *(--s);
}
