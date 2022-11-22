#define  FLI_1_0 
#include "errcodes.h"
#include "fli.h"
#include "imath.h"
#include "pjassert.h"

/* Function: pj_fli_read_head
 *
 *  Open the file, read in the FLI header and verify that it has the
 *  right magic number and generally is a fli file.
 *
 *  Returns Success and leaves file open in mode if it is a fli.
 *  Returns error code result and leaves file closed if it is not a
 *  fli or there was a file system error.
 */
Errcode
pj_fli_read_head(const char *title, Fli_head *flih,
		XFILE **pxf, enum XReadWriteMode mode)
{
	Errcode err;
	XFILE *xf;

	if (!pj_assert(pxf != NULL)) return Err_bad_input;

	*pxf = NULL;

	err = xffopen(title, &xf, mode);
	if (err != Success)
		return err;

	/* Read in fli header and check its magic number. */
	err = xffread(xf, flih, sizeof(*flih));
	if (err < Success)
		goto error;

	if (flih->type == FLIH_MAGIC) {
		flih->speed = pj_uscale_by(((Fhead_1_0 *)flih)->jiffy_speed,1000,70);
	}
	else if (flih->type != FLIHR_MAGIC) {
		err = Err_bad_magic;
		goto error;
	}

	/* do a little data checking */
	if (flih->bits_a_pixel == 0)
		flih->bits_a_pixel = 8;
	if (flih->aspect_dx == 0 || flih->aspect_dy == 0)
		flih->aspect_dx = flih->aspect_dy = 1;

	*pxf = xf;
	return Success;

error:
	if (err == Err_eof)
		err = Err_truncated;

	xffclose(&xf);
	return err;
}
