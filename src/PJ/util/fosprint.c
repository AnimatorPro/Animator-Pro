#include "formatf.h"

int
local_sprintf(char *buf, char *format,...)
/* just like ansii C.  Name is different to avoid conflict with prototype
   in stdio.h.  Watcom C has some bugs handling const declarations or
   we wouldn't have to do this.  */
{
Formatarg fa;

	start_formatarg(fa,format);
	while ((*buf++ = fa_getc(&fa)) != 0) {}
	end_formatarg(fa);
	return(fa.count - 1);
}
