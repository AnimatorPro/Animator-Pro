
#include "errcodes.h"
#include "memory.h"
#include "jfile.h"

Errcode pj_copydata(Jfile src, Jfile dest, LONG size)

/* copys data between two files reading from one writing to the other 
 * at current position for both files */
{
Errcode err;
char sbuf[256];	/* stack buffer */
char *buf;
LONG blocksize;

	blocksize = 32L*1024;	
	if(size < blocksize)
		blocksize = size;

	if(blocksize <= sizeof(sbuf))
		buf = sbuf;
	else if ((buf = pj_malloc(blocksize)) == NULL)
	{
		blocksize = sizeof(sbuf);
		buf = sbuf;
	}

	while(size > 0)
	{
		if(blocksize > size)
			blocksize = size;
		if(pj_read(src,buf,blocksize) < blocksize)
			goto error;
		if(pj_write(dest,buf,blocksize) < blocksize)
			goto error;
		size -= blocksize;
	}

	err = Success;
	goto done;
error:
	err = pj_ioerr();
done:
	if(buf != sbuf)
		pj_free(buf);
	return(err);
}
