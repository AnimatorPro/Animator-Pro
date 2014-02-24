#include "lfile.ih"
#include "formatf.h"

int lprintf(const char *format, ...)
{
Formatarg fa;
int c;

	start_formatarg(fa,format);
	while((c = fa_getc(&fa)) != 0)
		lputc(c,lstdout);
	end_formatarg(fa);
	return(fa.count - 1);
}
