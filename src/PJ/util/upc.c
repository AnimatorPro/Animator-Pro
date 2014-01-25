#include <ctype.h>
#include "util.h"

void upc(char *s)
{
register char c;

while (c = *s)
	{
	*s++ = toupper(c);
	}
}

