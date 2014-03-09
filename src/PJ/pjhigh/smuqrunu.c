#include "ftextf.h"
#include "pjbasics.h"
#include "commonst.h"
#include "softmenu.h"
#include "reqlib.h"

Boolean soft_ud_qreq_number(short *inum,short min,short max, 
		Errcode (*update)(void *data, SHORT val), void *vfuncdat,
		char *key, ...)
{
Boolean ret;
va_list args;

	va_start(args,key);
	ret = vsoft_qreq_number(inum,min,max,key,args,update,vfuncdat);
	va_end(args);
	return(ret);
}
