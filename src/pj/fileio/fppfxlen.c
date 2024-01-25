#include "filepath.h"

int path_prefix_len(char *path)

/* returns length of path prefix */
{
	return(pj_get_path_suffix(path) - path);
}
