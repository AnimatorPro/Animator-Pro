#include <ctype.h>

void upc(register char *s)
{
register char c;

while (c = *s)
	{
	*s++ = toupper(c);
	}
}

