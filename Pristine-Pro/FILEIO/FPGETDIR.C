#include "filepath.h"
Errcode get_dir(char *path)
{
	return(get_full_path("",path));
}
