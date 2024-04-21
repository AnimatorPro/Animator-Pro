#include <string.h>
#include "memory.h"
#include "util.h"

char *clone_string(char *s)
{
	unsigned long length = strlen(s) + 1;
	char *d = (char *)pj_malloc(length);

	if (d != NULL){
		strncpy(d, s, length);
	}
	
	return d;
}
