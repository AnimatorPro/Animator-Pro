#include "stdio.h"
#include "errcodes.h"
#include "ffile.h"

Errcode ffwriteoset(FILE *fp,void *buf, LONG oset,LONG size)
{
	if(fseek(fp,oset,SEEK_SET) != 0)
		goto error;
	if(fwrite(buf,1,size,fp) != size)
		goto error;

	return(Success);
error:
	return(ffile_error());
}
