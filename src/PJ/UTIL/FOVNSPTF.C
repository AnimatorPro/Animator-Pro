#include "formatf.h"

Errcode vnsprintf(char *buf, int maxlen, char *format, va_list args)
/* Var-args sprintf that won't go past maxlen.  Result _will_ be
   '\0' terminated.  */
{
Formatarg fa;

	copy_va_list(args,fa.args); /* see compiler.h */
	init_formatarg(&fa,format);
	return(fa_sprintf(buf,maxlen,&fa));
}
