#include <string.h>
#include "memory.h"
#include "util.h"

char *clone_string(char *s)
{
char *d;

if ((d = (char *)pj_malloc(strlen(s)+1)) != NULL)
	strcpy(d, s);
return(d);
}
