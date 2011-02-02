#include "fli.h"

Errcode pj_write_one_frame_fli(char *name, Flifile *flif,Rcel *screen)
/* writes out first frame and ring for one frame fli */
{
Errcode err;
Fli_frame *cbuf;

	if((err = pj_fli_cel_alloc_cbuf(&cbuf,screen)) < Success)
		goto error;
	if((err = pj_fli_add_frame1(name, flif, cbuf, screen)) < Success)
		goto error;
	if((err = pj_i_add_empty_ring(name,flif)) < Success)
		goto error;
error:
	pj_freez(&cbuf);
	return(err);
}
