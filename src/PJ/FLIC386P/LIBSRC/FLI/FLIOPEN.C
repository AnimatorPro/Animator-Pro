#include "errcodes.h"
#include "jfile.h"
#include "fli.h"

Errcode pj_fli_open(char *path, Flifile *flif, int jmode)
/************************************************************************* 
 * This routine is the first step in reading a fli.
 * Opens file in the mode specified and checks if it is a valid
 * fli file. It does some minor checks on data consistency,
 * Loads the Flifile and prepares it for use with Flifile routines.
 * Leaves file offset at an indeterminate position. 
 * 
 * Parameters:
 *		char 	*path;			File name of fli.
 *		Flifile *flif;			Structure to control an open fli.
 *		int		jmode;			JREADONLY in most cases.
 * Returns:
 *		Success (0) if all goes well, a negative error code if not.
 *		(see errcodes.h)
 *************************************************************************/
{
Errcode err;
Chunk_id fchunk;

	if((err = pj_fli_read_head(path,&flif->hdr,&flif->fd,jmode)) < 0)
		return(err);

	/* allways calc frame offsets unless there are no frames */

	if(flif->hdr.frame_count)
	{
		if(pj_read(flif->fd,&fchunk,sizeof(fchunk)) < sizeof(fchunk))
			goto jio_error;
		if(fchunk.type == FCID_FRAME)
		{
			flif->hdr.frame1_oset = sizeof(Fli_head);
			flif->hdr.frame2_oset = sizeof(Fli_head) + fchunk.size;
		}
		else if(fchunk.type == FCID_PREFIX)
		{
			flif->hdr.frame1_oset = fchunk.size + sizeof(Fli_head);
			if((err = pj_readoset(flif->fd,&fchunk,
								flif->hdr.frame1_oset,sizeof(fchunk))) < 0)
			{
				goto error;
			}
			if(fchunk.type != FCID_FRAME)
				goto corrupted_err;

			flif->hdr.frame2_oset = flif->hdr.frame1_oset + fchunk.size;
		}
		else
			goto corrupted_err;
	}
	return(Success);

corrupted_err:
	err = Err_corrupted;
	goto error;
jio_error:
	err = pj_ioerr();
error:
	if(err == Err_eof)
		err = Err_truncated;
	pj_fli_close(flif);
	return(err);
}
