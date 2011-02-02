#include "filepath.h"

char *pj_get_path_name(char *path)
/* returns pointer to last name in path */
{
char *name;

	name = path;
	for(;;)
	{
		switch(*path)
		{
			case 0:
				return(name);
			case DEV_DELIM:
			case DIR_DELIM:
				name = ++path;
				break;
			default:
				++path;
		}
	}
}
