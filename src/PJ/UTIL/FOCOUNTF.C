#include "formatf.h"

Errcode countf(char *fmt, ...)
/* Same parameters as printf.  Return length of formatted string. */
{
Formatarg fa;
Errcode err;

	start_formatarg(fa,fmt);
	err = fa_lenf(&fa);
	end_formatarg(fa);
	return(err);
}
