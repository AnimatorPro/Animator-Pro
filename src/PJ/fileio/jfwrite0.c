#include "jfile.h"
#include "errcodes.h"
#include "memory.h"

Errcode pj_write_zeros(Jfile file, LONG oset, ULONG bytes)
{
Errcode err;
char sbuf[256];	/* static buffer */
char *buf;
ULONG blocksize;

	if(bytes <= sizeof(sbuf))
	{
		blocksize = bytes;
		buf = sbuf;
		clear_mem(buf,blocksize);
	}
	else
	{
		if(bytes < 16*1024)
			blocksize = bytes;
		else
			blocksize = 16*1024;	

		if((buf = pj_zalloc(blocksize)) == NULL)
		{
			blocksize = sizeof(sbuf);
			buf = sbuf;
			clear_mem(sbuf,sizeof(sbuf));
		}
	}

	while(bytes > 0)
	{
		if((err = pj_writeoset(file,buf,oset,blocksize)) < Success)
			goto error;
		oset += blocksize;
		if(bytes < blocksize)
			blocksize = bytes;
		else
			bytes -= blocksize;
	}
	err = Success;
error:
	if(buf != &sbuf[0])
		pj_free(buf);
	return(err);
}
