#include "ftextf.h"

int snftextf(char *buf, int maxlen, char *fmt,...)

/* A sprintf() for the ftextf format type, If formats does not start
 * with "!%" formats is assumed to be an sprintf format type.
 * If formats does start with "!%", then the next argument is assumed
 * to be the string with replaceable arguments, followed by the arguments.  
 * "maxlen" defines the maximum size of the buffer to write into, including
 * a terminating '\0' */

{
va_list args;
char *formats;

	va_start(args,fmt);
	formats = ftext_format_type(&fmt,&args);
	maxlen = vnsftextf(buf,maxlen,formats,fmt,args);
	va_end(args);
	return(maxlen);
}
