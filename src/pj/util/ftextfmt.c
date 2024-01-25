#include "ftextf.h"

char *ftext_format_type(char **pfmt,va_list *pargs)

/* This item will check if the *pfmt begins with a "!%" and if it does
 * will set the va_list in (*pargs) up by one char * and assume the next item
 * is a Ftext style formated text followed by it's args. It will return the 
 * pointer to the "formats" string and adjust the pointer "pfmt" to point to 
 * the "text" argument and set the va_list to point to the first format 
 * argument */
{
char *fmt = *pfmt;

	if(*fmt++ == '!' && *fmt == '%')
	{
		*pfmt = va_arg((*pargs),char *);
		return(fmt);
	}
	return(NULL);
}
