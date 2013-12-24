#include "stdtypes.h"

int parse_to_semi(char **input, char *outstart,int maxlen)

/* put input up to (but not including) a semicolon into output.
   Skips leading white space and semicolons returns length of token
   gotten */
{
char *in = *input;
char *output = outstart;
char c;


	for (;;)
	{
		c = *in;
		switch(c)
		{
			case 0:
				return(0);
			case ' ':
			case '\t':
			case '\r':
			case '\n':
			case ';':
				++in;
				break;
			default:
				goto FIRSTC;
		}
	}

FIRSTC:
	for (;;)
	{
		c = *in;
		if(c == 0 || c == ';')
			break;
		if(--maxlen > 0)
			*output++ = c;
		++in;
	}
	*output = 0;
	*input = in;
	return(output - outstart);
}
