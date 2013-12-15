#include "fli.h"

LONG pj_fli_comp_frame1(void *cbuf,Rcel *this_screen,Flicomp comp_type)
/* given a screen. Makes a compressed first frame record from it and 
 * loads it in *cbuf */
{
	return(pj_fli_comp_cel(cbuf,NULL,this_screen,
						COMP_FIRST_FRAME,comp_type));
}
