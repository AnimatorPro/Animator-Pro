#include "filepath.h"

int get_drawer_len(char *path)

/* returns length of drawer part of path errcode */
{
	return(pj_get_path_name(path) - path);
}
