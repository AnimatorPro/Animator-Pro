#include "filepath.h"

Errcode make_file_path(char *drawer, char *name, char *outpath)
{
	return(add_subpath(drawer,pj_get_path_name(name),outpath));
}
