#include "ptrmacro.h"
#include "input.h"
#include "commonst.h"
#include "ftextf.h"

Errcode continu_box(char *fmt,...)
{
Errcode err;
va_list args;
char *formats;

	va_start(args, fmt);
	formats = ftext_format_type(&fmt,&args);
	err = varg_continu_box(formats,fmt,args,NULL);
	va_end(args);
	return(err);
}
