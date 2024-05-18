#include "stdio.h"
#include "errcodes.h"
#include "ffile.h"

Errcode ffread(FILE *fp,void *buf,LONG size)
{
	if(fread(buf,1,size,fp) != size)
		return(ffile_error());
	return(Success);
}
