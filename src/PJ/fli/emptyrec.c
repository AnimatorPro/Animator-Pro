#include "fli.h"


void pj_i_get_empty_rec(Fli_frame *frame)
/* Loads an empty, no change delta record into the *frame */
{
	memset(frame,0,sizeof(*frame));
	frame->size = sizeof(*frame);
	frame->type = FCID_FRAME;
}

#ifdef SLUFFED
Errcode pj_i_add_empty(char *name,Flifile *flif)
/* Same as fli_add_next() but writes a no change frame as the 
 * next frame for blank flis */
{
Fli_frame frame;

	pj_i_get_empty_rec(&frame);
	return(pj_i_add_next_rec(name,flif,&frame));
}
#endif  /* SLUFFED */

Errcode pj_i_add_empty_ring(char *name,Flifile *flif)
/* Same as fli_add_ring() but writes a no change frame as the 
 * next frame for blank flis */
{
Fli_frame frame;

	pj_i_get_empty_rec(&frame);
	return(pj_i_add_ring_rec(name,flif,&frame));
}
