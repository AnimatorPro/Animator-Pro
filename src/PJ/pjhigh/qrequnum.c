#ifdef SLUFFED
#include "ftextf.h"
#include "reqlib.h"

Boolean ud_qreq_number(short *inum,short min,short max, 
		Errcode (*update)(void *data, SHORT val), void *vfuncdat,
		char *hailing,...)
{
Boolean ret;
va_list args;
char *formats;

	va_start(args,hailing);
	formats = ftext_format_type(&hailing,&args);
	ret = varg_qreq_number(inum,min,max,update,vfuncdat,formats,hailing,args);
	va_end(args);
	return(ret);
}
#endif /* SLUFFED */
