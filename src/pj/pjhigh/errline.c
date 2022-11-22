#include <stdarg.h>
#include "jimk.h"
#include "commonst.h"
#include "errcodes.h"

Errcode errline(Errcode err,char *fmt,...)

/* note, will not report if Successful or Err_aborted or Err_reported */
{
char etext[ERRTEXT_SIZE];
va_list args;

	if(!get_errtext(err,etext))
		return(err);

	va_start(args,fmt);
	if(!fmt)
		fmt = empty_str;
	varg_continu_box(NULL,fmt,args,etext);
	va_end(args);
	return(Err_reported);
}
