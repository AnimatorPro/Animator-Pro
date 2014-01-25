#define  FLI_1_0 
#include "errcodes.h"
#include "fli.h"
#include "imath.h"

Errcode pj_fli_read_head(char *title, Fli_head *flih, Jfile *pfd,
						  	  int jmode)
/* open the file, read in the FLI header and verify that it has the,
 * right magic number and generally is a fli file.
 *
 * Returns Success and leaves file open in mode "jmode" if it is a fli.
 * Returns error code result and leaves
 * file closed if it is not a fli or there was a file system error. */
{
Jfile fd;
Errcode err;

	if((fd = pj_open(title, jmode)) == JNONE)
		goto jio_error;

	/* read in fli header and check it's magic number */
	if (pj_read(fd,flih,sizeof(*flih)) < sizeof(*flih) )
		goto jio_error;

	if(flih->type == FLIH_MAGIC)
	{
		flih->speed = pj_uscale_by(((Fhead_1_0 *)flih)->jiffy_speed,1000,70);
	}
	else if (flih->type != FLIHR_MAGIC)
	{
		err = Err_bad_magic;
		goto error;
	}
	/* do a little data checking */
	if(flih->bits_a_pixel == 0)
		flih->bits_a_pixel = 8;
	if(flih->aspect_dx == 0 || flih->aspect_dy == 0)
		flih->aspect_dx = flih->aspect_dy = 1;

	*pfd = fd;
	return(Success);

jio_error:
	err = pj_ioerr();
error:
	if(err == Err_eof)
		err = Err_truncated;
	pj_close(fd);
	*pfd = 0;
	return(err);
}
