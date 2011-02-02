/***** unused formating routines ****/

#include "formatf.h"
#include "lstdio.h"

/* standard ANSI library routines */

vsprintf(char *buf,char *format,va_list args)
{
Formatarg fa;

	copy_va_list(args,fa.args); /* see compiler.h */
	init_formatarg(&fa,format);
	while((*buf++ = fa_getc(&fa)) != 0);
	return(fa.count - 1);
}
/*********** some string and character functions ***********/

void to_upper(register UBYTE *s)

/* toupper a whole string */
{
register UBYTE c;

	while ((c = *s++) != 0)
	{
		if (islower(c))
			*(s-1) = _toupper(c);
	}
}
unsigned char lowc_char(unsigned char c)

/* a tolower function */
{
	if (c >= 'A' && c <= 'Z')
		c += 'a' - 'A';
	return(c);
}
