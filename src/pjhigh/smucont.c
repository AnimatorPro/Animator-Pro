#include "ftextf.h"
#include "reqlib.h"
#include "softmenu.h"

Errcode soft_continu_box(char *key,...)
{
	Errcode err;
	va_list args;
	char *text;
	char *formats;

	va_start(args, key);
	err = soft_load_ftext_type(key,&args,&formats,&text);
 	if(err >= Success) {
 		err = varg_continu_box(formats,text,args,NULL);
 	}
	va_end(args);

	smu_free_text(&text);
	return err;
}
