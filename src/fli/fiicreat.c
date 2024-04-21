#include "fli.h"
#include "jfile.h"

/* subroutine to fli_create() used in animator code
 * by create_flxfile() */
Errcode pj_i_create(char* path, Flifile* flif)
{
	Errcode err;

	err = xffopen(path, &flif->xf, XREADWRITE_CLOBBER);
	if (err < Success) {
		goto error;
	}

	pj_i_update_id(flif);
	flif->hdr.id.create_time = flif->hdr.id.update_time;
	flif->hdr.id.create_user = flif->hdr.id.update_user;
	flif->hdr.aspect_dx = flif->hdr.aspect_dy = 1; /* default value */
	flif->hdr.bits_a_pixel					  = 8;

	err = pj_i_flush_head(flif);
	if (err < Success) {
		goto error;
	}

	return Success;

error:
	pj_fli_close(flif);
	return err;
}
