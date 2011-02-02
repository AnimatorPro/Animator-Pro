#include "filepath.h"

char *pj_get_path_suffix(register char *path)

/* returns *path at start of suffix last name of path */
{
char *firstdot = NULL;

	for(;;)
	{
		switch(*path)
		{
			case 0: /* end of string */
				if(firstdot)
					return(firstdot);
				else
					return(path);
			case DIR_DELIM:
			case DEV_DELIM:
				firstdot = NULL;
				break;
			case SUFF_DELIM:
				if(!firstdot)
					firstdot = path;
				break;
		}
		++path;
	}
}
