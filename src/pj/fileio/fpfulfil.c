#ifdef SLUFFED
#include "filepath.h"

Errcode full_file_path(char *drawer, char *name, char *fullpath)
/* makes a fully expanded file path */
{
	return(full_path_name(drawer,pj_get_path_name(name),fullpath));
}
#endif /* SLUFFED */
