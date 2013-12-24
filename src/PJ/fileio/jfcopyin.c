#include "jfile.h"
#include "memory.h"
#include "errcodes.h"

Errcode copy_in_file(Jfile file, LONG bytes, LONG soff, LONG doff)

/* Move a piece of a file from one place to another. */
{
char sbuf[1024];	/* stack buffer */
char *buf;
LONG blocksize;
Boolean backwards;
Errcode err;
LONG fsize;
LONG expand;


	blocksize = 16*1024;	

	if((buf = pj_malloc(blocksize)) == NULL)
	{
		blocksize = sizeof(sbuf);
		buf = sbuf;
	}

	backwards = (doff > soff);
	if(backwards)	/* copy towards end of file? */
	{
		soff += bytes;	/* move pointers to end of copy block */
		doff += bytes;

		/* check size of whole file against destination offset */

		if((fsize = pj_seek(file,0,JSEEK_END)) < 0)
		{
			err = fsize;
			goto error;
		}
		/* Expand file if need be.  A bit inefficient since we may write
		 * to file twice. */

		if(bytes < blocksize)
			blocksize = bytes;

		expand = (doff - blocksize) - fsize;
		fsize = blocksize;

		while(expand > 0)
		{
			if(expand < fsize)
				fsize = expand;
			if((err = pj_write_ecode(file,buf,fsize)) < Success)
				goto error;
			expand -= fsize;
		}
	}

	while(bytes > 0)
	{
		if(bytes < blocksize)
			blocksize = bytes;
		if (backwards)
		{
			soff -= blocksize;
			doff -= blocksize;
		}
		if((err = pj_readoset(file,buf,soff,blocksize)) < Success)
			goto error;
		if((err = pj_writeoset(file,buf,doff,blocksize)) < Success)
			goto error;
		if(!backwards)
		{
			soff += blocksize;
			doff += blocksize;
		}
		bytes -= blocksize;
	}
	err = Success;
error:
	if(buf != sbuf)
		pj_free(buf);
	return(err);
}
