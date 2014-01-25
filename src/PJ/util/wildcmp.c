#include "wildlist.h"

#ifdef SLUFFED
static int match_substr(char *str, char *wild)
/* assumes wild starts on a normal char and not a '*' 0 or '?' */
{
int len = 0;

	for(;;)
	{
		if(*str++ != *wild++)
			return(0);
		++len;
		switch(*wild)
		{
			case 0:
			case '?':
			case '.':
			case '*':
				return(len);
		}
	}
}
int wildcmp(register char *wcard, register char *str)
/* does a case sensitive wild card match with a string
 *
 * . = Must match a '.' in str.
 * * = any string or no character.
 * ? = any single character.
 * anything else etc must match the character exactly. */
{
int match_star = 0;

	for(;;)
	{
		switch(*wcard)
		{
			case 0: /* end of wildcard */
			{
				if(match_star)
				{
					while(*str)
					{
						if(*str++ == '.') /* no dots allowed */
							return(0);
					}
				}
				else if(*str)
					return(0);

				return(1);
			}
			case '*':
				match_star = 1;
				break;
			case '?': /* anything but a dot will do */
			{
				if(*str == '.' || *str == 0)
					return(0); /* out of string or a dot, no match for ? */
				++str;
				break;
			}
			default:
			{
				if(match_star)
				{
					for(;;)
					{
						if(*str == 0) /* if out of string no match */
							return(0);
						if(*str == '.') /* only match star to next dot */
						{
							match_star = 0;
							break;
						}

						/* note match_star is re-used here for substring
						 * after star match length */

						if((match_star = match_substr(str,wcard)) != 0)
						{
							str += match_star;
							wcard += match_star;
							match_star = 0;
							goto next_wild;
						}
						++str;
					}
				}

				/* default: they must be equal or no match */
				if(*str != *wcard)
					return(0);
				++str;
				break;
			}
		}
		++wcard;
next_wild:
		continue;
	}
}
#endif /* SLUFFED */

int name_is_wild(char *name)
{
	char c;

	while((c = *name++) != 0)
	{
		if(c == '?' || c == '*')
			return TRUE;
	}
	return FALSE;
}
