#include "fli.h"

Errcode pj_fli_finish(char *name, /* name for error reporting
									 * if NULL no reports */
				   Flifile *flif, void *cbuf,
				   Rcel *last_screen, Rcel *work_screen)
{
Errcode err;
LONG endoset;

	if((endoset = pj_tell(flif->fd)) < 0)
		goto oseterr;

	if((err = pj_fli_seek_first(flif)) < Success)
		goto error;

	if((err = pj_fli_read_uncomp(name, flif, work_screen, cbuf, TRUE)) < Success)
		goto error;

	if((endoset = pj_seek(flif->fd, endoset, JSEEK_START)) < 0)
		goto oseterr;

	return(pj_fli_add_ring(name, flif, cbuf, last_screen, work_screen));

oseterr:
	err = endoset;
error:
	if(name)
		err = pj_fli_error_report(err,"Error writing FLC \"%s\"", name);
	return(err);
}

