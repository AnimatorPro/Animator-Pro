#include "filepath.h"
#include "util.h"

bool pj_name_in_path(char *path, char *name)
{
	return(*name && txtcmp(pj_get_path_name(path),name)==0);
}
