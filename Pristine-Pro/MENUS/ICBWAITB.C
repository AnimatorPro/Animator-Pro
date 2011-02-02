#include "errcodes.h"
#include "ptrmacro.h"
#include "ftextf.h"
#include "input.h"
#include "wordwrap.h"
#include "commonst.h"

Errcode put_wait_box(char *fmt,...)
{
va_list args;
Errcode ret;
char *formats;

	va_start(args,fmt);
	formats = ftext_format_type(&fmt,&args);
	ret =  varg_put_wait_box(formats, fmt,args);
	va_end(args);
	return(ret);
}
