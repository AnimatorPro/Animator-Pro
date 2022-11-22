#include "commonst.h"
#include "reqlib.h"

Errcode varg_continu_box(char *formats,char *text,va_list args,char *etext)
{
char *ctext[2];

	ctext[0] = continue_str;
	ctext[1] = NULL;
	return(tboxf_choice(icb.input_screen,formats,text,args,ctext,etext));
}
