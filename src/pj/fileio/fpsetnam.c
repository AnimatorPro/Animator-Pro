#include <string.h>
#include "filepath.h"

void pj_set_path_name(char *path, char *name)
{
	strcpy(pj_get_path_name(path),name);
}
