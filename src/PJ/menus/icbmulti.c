#ifdef SLUFFED
#include <stdarg.h>
#include "ptrmacro.h"
#include "input.h"
#include "ftextf.h"


Errcode multi_box(char **choices, char *fmt,...)

/* returns (index of choice button) + 1 */
{
va_list args;
char *formats;
Errcode ret;

	va_start(args,fmt);
	formats = ftext_format_type(&fmt,&args);
	ret = tboxf_choice(icb.input_screen,formats,fmt,args,choices,NULL);
	va_end(args);
	return(ret);
}
#endif /* SLUFFED */
