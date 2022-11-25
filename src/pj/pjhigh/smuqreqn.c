#include "reqlib.h"

Boolean soft_qreq_number(short *inum,short min,short max,char *key,...)
{
Boolean ret;
va_list args;

	va_start(args,key);
	ret = vsoft_qreq_number(inum, min, max, key, args, NULL, NULL);
	va_end(args);
	return(ret);
}

