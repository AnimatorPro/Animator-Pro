#include <ctype.h>
#include "util.h"

int txtncmp(const char *as, const char *bs, int len)
/* like strcmp but case insensitive */
{
register UBYTE a, b;
const char *maxas;

	maxas = as + len;

	for(;;)
	{
		if(as >= maxas)
			return(0);

		a = *as++;
		b = *bs++;
		if(a == 0)
			break;

		if (islower(a))
			a = _toupper(a);
		if (islower(b))
			b = _toupper(b);
		if (a != b)
			break;
	}
	return(a-b);
}
