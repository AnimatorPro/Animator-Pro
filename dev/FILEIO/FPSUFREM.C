#include "filepath.h"

void remove_suffix(char *path)

/* "removes" any suffix from path by null terminating */
{
	*pj_get_path_suffix(path) = 0;
}
