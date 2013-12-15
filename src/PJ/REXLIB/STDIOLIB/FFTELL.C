#include "stdio.h"
#include "errcodes.h"
#include "ffile.h"

LONG fftell(FILE *fp)
{
LONG oset;

	if((oset = ftell(fp)) < 0)
		return(ffile_error());
	return(oset);
}
