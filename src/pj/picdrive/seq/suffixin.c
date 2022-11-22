
#include "stdtypes.h"

static int to_upper(char a)
/* Convert lower case character to upper case.  Leave other characters
 * unchanged. */
{
if (a >= 'a' && a <= 'z')
	return(a + 'A' -  'a');
else
	return(a);
}

static txtcmp(char *a, char *b)
/* compare two strings ignoring case */
{
char aa,bb;

for (;;)
	{
	aa = to_upper(*a++);	/* fetch next characters converted to upper case */
	bb = to_upper(*b++);
	if (aa != bb)			/* if not equals return difference */
		return(aa-bb);
	if (aa == 0)
		return(0);
	}
}

Boolean suffix_in(char *string, char *suff)
{
string += strlen(string) - strlen(suff);
return( txtcmp(string, suff) == 0);
}

