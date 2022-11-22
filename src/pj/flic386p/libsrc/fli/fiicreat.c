#include "jfile.h"
#include "fli.h"

Errcode pj_i_create(char *path, Flifile *flif)

/* subroutine to fli_create() used in animator code 
 * by create_flxfile() */
{
Errcode err;

	if((flif->fd = pj_create(path,JREADWRITE)) == JNONE)
		goto jio_error;

	pj_i_update_id(flif);
	flif->hdr.id.create_time = flif->hdr.id.update_time;
	flif->hdr.id.create_user = flif->hdr.id.update_user;
	flif->hdr.aspect_dx = flif->hdr.aspect_dy = 1;  /* default value */
	flif->hdr.bits_a_pixel = 8;
	if((err = pj_i_flush_head(flif)) < Success)
		goto error;

	return(Success);
jio_error:
	err = pj_ioerr();
error:
	pj_fli_close(flif);
	return(err);
}
