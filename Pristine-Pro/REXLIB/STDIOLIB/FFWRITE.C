#include "stdio.h"
#include "errcodes.h"
#include "ffile.h"

Errcode ffwrite(FILE *fp,void *buf,LONG size)
{
	if(fwrite(buf,1,size,fp) != size)
		return(ffile_error());
	return(Success);
}
