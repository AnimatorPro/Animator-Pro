#include "lfile.ih"
#include "formatf.h"

int lfprintf(LFILE *f, char *fmt, ...)
{
va_list varg;
Formatarg fa;
char c;

start_formatarg(fa,fmt);
while ((c = fa_getc(&fa)) != 0)
	if (lputc(c,f) == LEOF)
		return(Err_write);
end_formatarg(fa);
return(fa.count - 1);
}
