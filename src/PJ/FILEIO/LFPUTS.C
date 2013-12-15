#include "errcodes.h"
#include "lfile.ih"

lfputs(char *s,LFILE *f)
{
int c;

	while ( (c  = *s++ ) != 0)
	{
		if (lputc(c,f) < Success)
			return(LEOF);
	}
	return(Success);
}
