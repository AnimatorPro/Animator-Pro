#include "util.h"

void tr_string( char *string, char in, char out)
/*
 * Change all occurences of character "in" to character "out"
 */
{
char c;

while ((c = *string)!=0)
	{
	if ( c == in )
		*string = out;
	string++;
	}
}

