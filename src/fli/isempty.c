#include "fli.h"

bool pj_i_is_empty_rec(Fli_frame *frame)
/* returns TRUE if the *frame is an empty delta record */
{
	return(frame->size <= (long)sizeof(Fli_frame));
}
