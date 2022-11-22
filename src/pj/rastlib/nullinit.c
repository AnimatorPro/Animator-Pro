#include "memory.h"
#include "rastlib.h"
#include "libdummy.h"

void pj_init_null_rastlib(Rastlib *lib)
/* Set unimplemented parts of a raster library to functions that
   do nothing. */
{
	pj_stuff_pointers(pj_vdo_nutin,lib,NUM_LIB_CALLS);
	lib->close_raster = (rl_type_close_raster)pj_errdo_success;
	lib->cget_dot = (rl_type_cget_dot)pj_rcdo_nutin;
	lib->get_dot = (rl_type_get_dot)pj_rcdo_nutin;
}
