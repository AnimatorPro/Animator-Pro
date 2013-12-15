#include "rastlib.h"

void *pj_get_null_lib(void)
{
static Rastlib null_lib;
static UBYTE loaded = FALSE;

	if(!loaded)
	{
		pj_init_null_rastlib(&null_lib);
		loaded = TRUE;
	}
	return(&null_lib);
}
