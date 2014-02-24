#include "errcodes.h"
#include "lfile.ih"

unsigned lfwrite(const void *buf, unsigned size, unsigned count, LFILE *f)
/* This one isn't optimized yet.   Just in terms of lputc */
{
int icount, isize;
const char *pt = buf;

for (icount = 0;  icount<count; icount++)
	{
	for (isize=size; --isize>=0; )
		{
		if (lputc(*pt++,f) < Success)
			goto OUT;
		}
	}
OUT:
return(icount);
}
