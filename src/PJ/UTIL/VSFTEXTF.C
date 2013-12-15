#include "ftextf.h"

int vsftextf(char *buf, char *formats,char *text, va_list args)

/* A vsprintf() for the ftextf format type, If formats is NULL it will act as
 * vsprintf, If formats is present and does not start with "!%" formats is 
 * assumed to be a string that will be copied into buf and it MUST NOT have any
 * replaceable argument descriptors "![xx]".  If formats does start with "!%", 
 * then the next argument is assumed to be the string with replaceable ftext 
 * arguments. */
{
Ftextfarg fa;

	copy_va_list(args,fa.fa.args);
	init_eitherfarg(&fa,formats,text);
	while((*buf++ = fa_getc(&fa.fa)) != 0);
	end_formatarg(fa.fa);
	return(fa.fa.count - 1);
}
