#ifdef USE_LFILE
	#include "lstdio.h"
#else
	#include <stdio.h>
#endif
#include "errcodes.h"
#include "ffile.h"

Errcode ffseek(FILE *fp,LONG oset,int how)
{
	if(fseek(fp,oset,how))
		return(ffile_error());
	return(Success);
}
