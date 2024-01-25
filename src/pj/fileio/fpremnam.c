#include "filepath.h"

void remove_path_name(char *path)
/* truncates last name on path, leaves trailing '\' */
{
	*pj_get_path_name(path) = 0;
}
