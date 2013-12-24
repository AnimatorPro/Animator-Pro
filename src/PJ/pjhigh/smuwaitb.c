#include "pjbasics.h"
#include "softmenu.h"
#include "ftextf.h"

Errcode soft_put_wait_box(char *key,...)
{
Errcode ret;
va_list args;
char *text;
char *formats;

	va_start(args,key);
 	if((ret = soft_load_ftext_type(key,&args,&formats,&text)) >= Success)
		ret = varg_put_wait_box(formats,text,args);
	va_end(args);
	smu_free_text(&text);
	return(ret);
}
