#include "jfile.h"

Errcode write_gulp(char *name, void *buf, long size)
/* Write out a file of known size all at once returns Ecode */
{
Jfile f;

	if((f = pj_create(name,JWRITEONLY))==JNONE)
		goto io_error;
	if (pj_write(f, buf, size) < size)
	{
		pj_close(f);
		pj_delete(name);
		goto io_error;
	}
	pj_close(f);
	return(Success);
io_error:
	return(pj_ioerr());
}
