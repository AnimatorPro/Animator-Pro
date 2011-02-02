#include "rastcall.h"
#include "rastlib.h"

void pj_grc_load_rwcalls(Rastlib *lib)
{
	pj_grc_load_compcalls(lib);
	pj_grc_load_dcompcalls(lib);
}
