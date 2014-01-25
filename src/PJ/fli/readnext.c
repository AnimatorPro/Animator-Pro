#include "fli.h"
#include "memory.h"

Errcode pj_fli_read_next(char *fname,
						Flifile *flif,
						Rcel *fscreen,
						Boolean colors)
/*************************************************************************
 * Read in next frame from a fli onto a Rcel (which may be displayable or
 * not).  This is the main function to use if you with to step through a
 * fli one frame at a time.
 *
 * Parameters:
 *		char	*fname; 	Name of fli file.  Will be used in reporting
 *							error messages if provided.  If NULL errors
 *							will not be reported (but will be available
 *							in the return value.
 *		Flifile *flif;		A Flifile initialized with pj_fli_open().
 *		Rcel	*fscreen;	Screen to update.  Should contain previous
 *							frame of fli.
 *		int 	colors; 	Non-zero if hardware color map should be
 *							changed if there's color info in the fli.
 * Returns:
 *		Success (0) if all goes well, a negative error code if not.
 *		(see errcodes.h)
 *************************************************************************/
{
Fli_frame *ff;
Errcode err;

	if((err = pj_fli_alloc_cbuf(&ff,flif->hdr.width,flif->hdr.height,COLORS)) < 0)
		goto error;
	if((err = pj_fli_read_uncomp(fname, flif, fscreen, ff, colors)) < 0)
		goto error;
	goto out;

error:
	if(fname)
		err = pj_fli_error_report(err,"Bad frame in \"%s\"",fname);
out:
	pj_gentle_free(ff);
	return(err);
}
