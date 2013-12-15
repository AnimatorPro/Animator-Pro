#include "errcodes.h"
#include "ptrmacro.h"
#include "ftextf.h"
#include "input.h"

Errcode boxf(char *fmt,...)
/* this puts up a formated textbox for debugging etc bypassing input calls 
 * this only does printf() style formats */
{
Errcode err;
va_list args;

	va_start(args, fmt);
	err = tboxf(icb.input_screen,fmt,args);
	va_end(args);
	return(err);
}
