
/* foolwcc2.c - Watcom C compiler likes it better if we keep this function
   to convert between a long and a pointer in a separate file because
   we're deliberately fooling it's type checking on make_ptr... */

#include "jimk.h"

void *
long_to_pt(l)
unsigned long l;
{
unsigned segment, offset;

offset = (l&15);
l >>= 4;
segment = l;
return(make_ptr(offset, segment));
}

