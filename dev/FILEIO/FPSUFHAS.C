#include "filepath.h"

Boolean has_a_suffix(char *path)
{
	return(*pj_get_path_suffix(path) != 0);
}
