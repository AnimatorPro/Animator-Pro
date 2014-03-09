#include "reqlib.h"
#include "softmenu.h"

Boolean soft_qreq_string(char *strbuf,int bufsize,char *key,...)
{
Boolean ret;
va_list args;
char *text;
char *formats;

	va_start(args,key);
 	if(soft_load_ftext_type(key,&args,&formats,&text) < Success)
		ret = FALSE;
	else
		ret = varg_qreq_string(strbuf,bufsize,formats,text,args);

	smu_free_text(&text);
	va_end(args);
	return(ret);
}
