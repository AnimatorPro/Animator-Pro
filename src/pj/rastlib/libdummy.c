#include "rastcall.h" /* to get typedef for Pixel */
#include "errcodes.h"
#include "libdummy.h"

void pj_vdo_nutin(void) { return; }

#ifdef SLUFFED
SHORT pj_sdo_nutin(void) { return(0); }
#endif /* SLUFFED */

Pixel pj_rcdo_nutin(void) { return(0); }
Errcode pj_errdo_unimpl(void) { return(Err_unimpl); }
Errcode pj_errdo_success(void) { return(Success); }

