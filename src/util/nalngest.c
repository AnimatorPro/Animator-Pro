#include <string.h>
#include "linklist.h"

int longest_name(Names *names)
{
int acc, len;

acc = 0;
while (names != NULL)
	{
	len = strlen(names->name);
	if (len > acc)
		acc = len;
	names = names->next;
	}
return(acc);
}
