#include "rastcall.ih"
#include "errcodes.h"
#include "memory.h"

void pj_set_grc_calls(void *inlib)

/* sets vectors to generic calls wherever the lib has NULL vectors */
{
	pj_load_array_nulls((void **)pj_get_grc_lib(),(void **)inlib,NUM_LIB_CALLS);
}
