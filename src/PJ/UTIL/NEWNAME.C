
#include "errcodes.h"
#include "memory.h"
#include "linklist.h"

Errcode new_name(Names **pname, char *s, Names **plist)
{
Names *n;
int len;

len = strlen(s)+1;
if ((*pname = n = pj_malloc(sizeof(*n)+len)) == NULL)
	return(Err_no_memory);
strcpy((n->name = (char *)(n+1)), s);
n->next = *plist;
*plist = n;
return(len);
}

