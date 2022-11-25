#include "errcodes.h"
#include "fli.h"

/* Function: pj_fli_open
 *
 *  This routine is the first step in reading a fli.
 *  Opens file in the mode specified and checks if it is a valid
 *  fli file.  It does some minor checks on data consistency,
 *  loads the Flifile and prepares it for use with Flifile routines.
 *  Leaves file offset at an indeterminate position.
 *
 *  path - file name of fli.
 *  flif - structure to control an open fli.
 *  mode - XREADONLY in most cases.
 */
Errcode
pj_fli_open(char *path, Flifile *flif, enum XReadWriteMode mode)
{
Errcode err;
Chunk_id fchunk;

	err = pj_fli_read_head(path, &flif->hdr, &flif->xf, mode);
	if (err < Success)
		return err;

	/* allways calc frame offsets unless there are no frames */

	if(flif->hdr.frame_count)
	{
		err = xffread(flif->xf, &fchunk, sizeof(fchunk));
		if (err < Success)
			goto error;

		if(fchunk.type == FCID_FRAME)
		{
			flif->hdr.frame1_oset = sizeof(Fli_head);
			flif->hdr.frame2_oset = sizeof(Fli_head) + fchunk.size;
		}
		else if(fchunk.type == FCID_PREFIX)
		{
			flif->hdr.frame1_oset = fchunk.size + sizeof(Fli_head);

			err = xffreadoset(flif->xf, &fchunk,
					flif->hdr.frame1_oset, sizeof(fchunk));
			if (err < Success)
				goto error;

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

error:
	if (err == Err_eof)
		err = Err_truncated;
	pj_fli_close(flif);
	return(err);
}
