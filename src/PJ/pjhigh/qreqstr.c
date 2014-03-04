#ifdef WITH_POCO
#include "ftextf.h"
#include "pjbasics.h"
#include "commonst.h"


Boolean qreq_string(char *strbuf,int bufsize,char *hailing,...)
{
Boolean ret;
va_list args;
char *formats;

	va_start(args,hailing);
	formats = ftext_format_type(&hailing,&args);
	ret = varg_qreq_string(strbuf,bufsize,formats,hailing,args);
	va_end(args);
	return(ret);
}
#endif /* WITH_POCO */
