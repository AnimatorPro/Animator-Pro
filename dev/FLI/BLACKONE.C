#include "fli.h"

Errcode pj_i_add_black1(char *name,Flifile *flif,Rcel *screen)
/* same as fli_add_frame1() but writes the first frame as a solid color
 * black frame for blank flis includes color palette contained in screen */
{
Fli_frame *cbuf;
Errcode err;

	if((err = pj_fli_alloc_cbuf(&cbuf,16,16,screen->cmap->num_colors)) < Success)
		goto error;
	pj_fli_comp_rect((Fli_frame *)cbuf,NULL,screen,NULL,TRUE,COMP_BLACK_FIRST,
				   flif->comp_type);

	err = pj_i_add_frame1_rec(name,flif,(Fli_frame *)cbuf);
	pj_free(cbuf);
error:
	return(err);
}
