#ifdef SLUFFED
#include "ptrmacro.h"
#include "input.h"
#include "commonst.h"
#include "ftextf.h"


Boolean yes_no_box(char *fmt,...)
{
va_list args;
Boolean ret;
char *formats;

	va_start(args,fmt);
	formats = ftext_format_type(&fmt,&args);
	ret = varg_yes_no_box(formats,fmt,args);
	va_end(args);
	return(ret);
}
#endif /* SLUFFED */
