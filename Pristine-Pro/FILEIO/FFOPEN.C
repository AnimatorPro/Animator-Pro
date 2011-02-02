#ifdef USE_LFILE
	#include "lstdio.h"
#else
	#include <stdio.h>
#endif
#include "errcodes.h"
#include "ffile.h"

Errcode ffile_error()
{
#ifdef LSTDIO_H
	return(lerrno);
#else
	return(Err_stdio);
#endif
}
void ffclose(FILE **pfp)
{
	if(*pfp)
		fclose(*pfp);
	*pfp = NULL;
}
Errcode ffopen(char *path, FILE **pfp, char *fmode)
{
	if(NULL == (*pfp = fopen(path,fmode)))
		return(ffile_error());
	return(Success);
}
