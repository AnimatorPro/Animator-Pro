#include "commonst.h"
#include "reqlib.h"

void cleanup_wait_box(void)
{
	cleanup_wait_wndo(icb.input_screen);
}
Errcode varg_put_wait_box(char *formats, char *text,va_list args)
{
	return(va_wait_wndo(icb.input_screen,please_wait_str,formats,text,args));
}
