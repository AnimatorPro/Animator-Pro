#include "errcodes.h"
#include "rcel.h"
#include "fli.h"

Errcode pj_fli_read_uncomp(char *fname, Flifile *flif, Rcel *fscreen,
					   Fli_frame *ff, int colors)
/*************************************************************************
 * Read in next frame from a fli onto a Rcel.  See also the related but
 * simpler pj_fli_read_next().
 *
 * Parameters:
 *		char	*fname; 	Name of fli file.  Will be used in reporting
 *							error messages if provided.  If NULL errors
 *							will not be reported (but will be available
 *							in the return value.
 *		Flifile *flif;		A Flifile initialized with pj_fli_open().
 *		Rcel	*fscreen;	Screen to update.  Should contain previous
 *							frame of fli.
 *		Fli_frame *ff;		Buffer for compressed frame.  Should be
 *							initialized previously with pj_fli_alloc_cbuf()
 *							or pj_fli_cel_alloc_cbuf().
 *		int 	colors; 	Non-zero if hardware color map should be
 *							changed if there's color info in the fli.
 * Returns:
 *		Success (0) if all goes well, a negative error code if not.
 *		(see errcodes.h)
 * Maintenance:
 *		9/08/91 - (Jim) Converted call to pj_fli_uncomp_rect() to
 *				  pj_fli_uncomp_frame() because the x/y offset calculation
 *				  here was messing up the fliclib, and everyone else
 *				  was sending it data such that x/y ended up 0 anyway.
 *************************************************************************/
{
long size_left;
Errcode err;


	if (pj_read(flif->fd, ff, sizeof(*ff)) < (long)sizeof(*ff))
		goto jio_error;

	if (ff->type != FCID_FRAME)
	{
		err = Err_bad_magic;
		goto error;
	}
	if (ff->size >=  pj_fli_cbuf_size(flif->hdr.width,flif->hdr.height,COLORS))
	{
		err = Err_corrupted;
		goto error;
	}
	size_left = ff->size - sizeof(*ff);
	if (pj_read(flif->fd,ff+1,size_left) < size_left)
		goto jio_error;
	if(fscreen)
	{
		pj_fli_uncomp_frame(fscreen,ff,colors);
	}
	return(Success);

jio_error:
	err = pj_ioerr();
error:
	if(err == Err_eof)
		err = Err_truncated;
	if(fname)
		err = pj_fli_error_report(err,"Bad frame in \"%s\"",fname);
	return(err);
}
