#include "errcodes.h"
#include "jfile.h"

Errcode read_gulp(const char *name, void *buf, long size)
/* Read in a file of known size all at once. */
{	
Jfile f;

	if ((f = pj_open(name,0)) == JNONE)
		goto io_error;
	if (pj_read(f, buf, size) < size)
	{
		pj_close(f);
		goto io_error;
	}
	pj_close(f);
	return(Success);
io_error:
	return(pj_ioerr());
}
