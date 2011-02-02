#include "formatf.h"

Errcode vcountf(char *fmt, va_list args)
/* Same parameters as printf.  Return length of formatted string. */
{
Formatarg fa;
Errcode err;

	copy_va_list(args,fa.args);
	init_formatarg(&fa,fmt);
	err = fa_lenf(&fa);
	end_formatarg(fa);
	return(err);
}
