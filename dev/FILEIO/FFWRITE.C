#ifdef USE_LFILE
	#include "lstdio.h"
#else
	#include <stdio.h>
#endif
#include "errcodes.h"
#include "ffile.h"

Errcode ffwrite(FILE *fp,void *buf,LONG size)
{
	if(fwrite(buf,1,size,fp) != size)
		return(ffile_error());
	return(Success);
}
