#include "jfile.h"
#include "memory.h"

Errcode pj_cpfile(char *source, char *dest, char **perrfile)

/* pj_cpfile copies source to destination.  Does not report error
   but if there is one returns the error code and sets **perrfile
   to either source or dest (depending where error was). */
{
Jfile in,out;
unsigned size;
char sbuf[256];	/* stack buffer */
char *buf;
long blocksize;
Errcode err;

	*perrfile = NULL;
	blocksize = PJ_COPY_FILE_BLOCK;	
	if ((buf = trd_laskmem(blocksize)) == NULL)
	{
		blocksize = sizeof(sbuf);
		buf = sbuf;
	}

	out = NULL; /* for jclose() */

	if ((in = pj_open(source, 0)) == JNONE)
		goto read_err;

	if((out = pj_create(dest, JWRITEONLY)) == JNONE)
		goto write_err;

	for (;;)
	{
#ifdef LATER
/* right now jio_err() doesn't seem to be returning EOF properly... */
		if( (size = pj_read(in, buf, blocksize)) < blocksize
			 && pj_ioerr() != Err_eof)
		{
			goto read_err;
		}
#else
		size = pj_read(in, buf, blocksize);
#endif
		if (pj_write(out,buf,size) < size)
			goto write_err;
		if (size < blocksize)
			break;
	}

	err = Success;
	goto EXIT;
write_err:
	*perrfile = dest;
	goto io_error;
read_err:
	*perrfile = source;
io_error:
	err = pj_ioerr();
	goto EXIT;
EXIT:
	if(buf != sbuf)
		trd_freemem(buf);
	pj_close(in);
	pj_close(out);
	return(err);
}
