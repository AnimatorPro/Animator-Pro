#include "stdtypes.h"
#include "errcodes.h"
#include "stdio.h"

Errcode unpic_line(FILE *f, UBYTE *buf, int len)
/* unpic_line() - read in a single mac pic (or amiga ILBM) line 
   from a buffered file and decompress it into buf argument.  Decompressed
   length is len bytes.  */
{
UBYTE *p;
char b;
UBYTE d;
int count;

/* first uncompress it into the buffer mon */
p = buf;
while (len > 0)
	{
	if ((b = getc(f)) < 0)	/* it's a run */
		{
		d = getc(f);
		count = -b;
		count += 1;
		len -= count;
		while (--count >= 0)
			*p++ = d;
		}
	else
		{
		count = b;
		count += 1;
		if (fread(p, 1, count, f) != count)
			{
			return(Err_truncated);
			}
		p += count;
		len -= count;
		}
	}
return((len == 0) ? Success : Err_format);
}
