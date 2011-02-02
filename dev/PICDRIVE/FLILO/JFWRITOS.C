
#include "errcodes.h"
#include "jfile.h"

Errcode pj_jwriteoset(Jfile f,void *buf, LONG oset,LONG size)

/* combo seek and read */
{
	if(pj_seek(f,oset,JSEEK_START) < 0)
		goto jio_error;
	if(pj_write(f, buf, size) < size)
		goto jio_error;

	return(Success);
jio_error:
	return(pj_ioerr());
}
