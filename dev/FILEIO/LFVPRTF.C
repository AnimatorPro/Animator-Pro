#include "formatf.h"
#include "lfile.ih"

int lvfprintf(LFILE *f, char *fmt, va_list args)
{
Formatarg fa;
char c;

	copy_va_list(args,fa.args); /* see compiler.h */
	init_formatarg(&fa,fmt);
	while((c = fa_getc(&fa)) != 0)
		{
		if (lputc(c,f) < Success)
			break;
		}
	return(fa.count - 1);
}
