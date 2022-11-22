#include "stdio.h"
#include "errcodes.h"
#include "ffile.h"

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
