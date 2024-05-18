#include "ftextf.h"
#include "reqlib.h"
#include "softmenu.h"

bool soft_yes_no_box(char *key,...)
{
int ret;
va_list args;
char *text;
char *formats;

	va_start(args,key);
 	if(soft_load_ftext_type(key,&args,&formats,&text) < Success)
		ret = false;
	else
		ret = varg_yes_no_box(formats,text,args);

	va_end(args);
	smu_free_text(&text);
	return(ret);
}
