#include <ctype.h>
#include "util.h"

void upc(char *s)
{
register char c;

while ((c = *s) != '\0')
	{
	*s++ = toupper(c);
	}
}

