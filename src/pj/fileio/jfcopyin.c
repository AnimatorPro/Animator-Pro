#include "jfile.h"
#include "memory.h"
#include "errcodes.h"

/* Move a piece of a file from one place to another. */
Errcode
copy_in_file(XFILE *xf, LONG bytes, LONG soff, LONG doff)
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
		fsize = xffseek_tell(xf, 0, XSEEK_END);
		if (fsize < 0) {
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

			err = xffwrite(xf, buf, fsize);
			if (err < Success)
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

		err = xffreadoset(xf, buf, soff, blocksize);
		if (err < Success)
			goto error;

		err = xffwriteoset(xf, buf, doff, blocksize);
		if (err < Success)
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
