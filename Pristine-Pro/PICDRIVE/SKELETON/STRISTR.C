/*****************************************************************************
 * stristr.c - some code to implement stristr().
 ****************************************************************************/

enum {FALSE, TRUE};
#define NULL 0

static void strlwr(char *str)
{
	while (*str) {
		if (*str >= 'a' && *str <= 'z')
			*str -= 32;
		++str;
	}
}

static int substreq(char *str, char *substr)
{
	while (*substr) {
		if (*substr++ != *str++)
			return FALSE;
	}
	return TRUE;
}


char *stristr(char *string, char *pattern)
{
	char patc1 = *pattern;
	char *pstr	= string;

	strlwr(string);
	strlwr(pattern);

	while (*pstr) {
		if (*pstr == patc1
		 && substreq(pstr, pattern))
			return pstr;
		else
			++pstr;
	}

	return NULL;
}
