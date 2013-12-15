#include "ftextf.h"

int sftextf(char *buf, char *fmt,...)

/* A sprintf() for the ftextf format type, If formats does not start
 * with "!%" formats is assumed to be an sprintf format type.
 * If formats does start with "!%", then the next argument is assumed
 * to be the string with replaceable arguments, followed by the arguments. */
{
va_list args;
char *formats;
int len;

	va_start(args,fmt);
	formats = ftext_format_type(&fmt,&args);
	len = vsftextf(buf,formats,fmt,args);
	va_end(args);
	return(len);
}
