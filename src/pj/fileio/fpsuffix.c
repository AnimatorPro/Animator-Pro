#include <string.h>
#include "filepath.h"

char *fix_suffix(char *path)
/* truncates to max size allowed (maxlen + delimiter) and returns pointer 
 * to suffix which is a pointer to the string's terminating 0 if there is
 * no suffix */
{
char *sfix;

	if(strlen(sfix = pj_get_path_suffix(path)) > MAX_SUFFIX_NAME_LEN+1)
	{
		sfix[MAX_SUFFIX_NAME_LEN+1] = 0;
	}
	return(sfix);
}
