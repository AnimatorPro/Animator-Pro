#ifdef USE_LFILE
	#include "lstdio.h"
#else
	#include <stdio.h>
#endif
#include "errcodes.h"
#include "ffile.h"

LONG fftell(FILE *fp)
{
LONG oset;

	if((oset = ftell(fp)) < 0)
		return(ffile_error());
	return(oset);
}
