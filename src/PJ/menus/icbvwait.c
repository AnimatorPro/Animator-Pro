#include "errcodes.h"
#include "ptrmacro.h"
#include "ftextf.h"
#include "input.h"
#include "wordwrap.h"
#include "commonst.h"



void cleanup_wait_box()
{
	cleanup_wait_wndo(icb.input_screen);
}
Errcode varg_put_wait_box(char *formats, char *text,va_list args)
{
	return(va_wait_wndo(icb.input_screen,please_wait_str,formats,text,args));
}
