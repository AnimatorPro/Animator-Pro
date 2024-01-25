#include "stdio.h"
#include "errcodes.h"
#include "ffile.h"

Errcode ffseek(FILE *fp,LONG oset,int how)
{
	if(fseek(fp,oset,how))
		return(ffile_error());
	return(Success);
}
