#include "fli.h"

Errcode pj_i_read_uncomp1(char *fname, /* name of file for error report 
										  * NULL == no reporting */
					   Flifile *flif,
					   Rcel *fscreen,	/* optional screen to update */
					   Fli_frame *ff,  /* frame buffer */
					   Boolean colors) /* wait for vblank and update 
					   			        * hardware palette? */

/* seeks to and reads first frame into *ff using fii_read_uncomp()
 * assuming frame1_oset in the Flifile header is valid will update *fscreen
 * if *fscreen is non-NULL */
{
Errcode err;
	if((err = pj_fli_seek_first(flif)) < 0)
		return(err);
	return(pj_fli_read_uncomp(fname,flif,fscreen,ff,colors));
}
