#ifdef USE_LFILE
	#include "lstdio.h"
#else
	#include <stdio.h>
#endif
#include "errcodes.h"
#include "ffile.h"

Errcode ffreadoset(FILE *fp,void *buf, LONG oset,LONG size)
{
	if(fseek(fp,oset,SEEK_SET) != 0)
		goto error;
	if(fread(buf,1,size,fp) != size)
		goto error;

	return(Success);
error:
	return(ffile_error());
}
