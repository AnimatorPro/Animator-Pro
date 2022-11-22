#include "torture.h"

/* This file contains little routines that serve little purpose except
 * to provide missing symbols to link with the PJ library code. */

boxf(char *format, ...)
/* This is supposed to be a debugging printf essentially. */
{
puts(format);
}

short	old_vmode;

void old_video()
{
pj_set_vmode(old_vmode);
}
