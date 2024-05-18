#define REXLIB_INTERNALS
#include "rexlib.h"
#include "rexlib.h"

extern Hostlib _a_a_loadpath;

char *pj_get_load_path()
{
	return((char *)(_a_a_loadpath.next));
}

