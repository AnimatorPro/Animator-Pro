#include <string.h>
#include "filepath.h"

static Boolean has_dir_chars(char *path)
/* Returns TRUE if there is a directory in the path name. */
{
	char c;

	while ((c = *path++) != 0)
		{
		if (c == ':' || c == '\\' ||  c == '/')
			return TRUE;
		}
	return FALSE;
}

Errcode full_path_name(char *drawer,char *subpath,char *fullpath)
/* makes fully expanded concatenation of a drawer and subpath */
{
Errcode err;

	if (has_dir_chars(subpath))
		strcpy(fullpath, subpath);
	else
		{
		if((err = add_subpath(drawer, subpath, fullpath)) < 0)
			return(err);
		}
	return(get_full_path(fullpath,fullpath));
}
