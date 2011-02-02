#include <stdarg.h>
#include "ptrmacro.h"
#include "input.h"
#include "commonst.h"


Boolean varg_yes_no_box(char *formats,char *text,va_list args)
{
char *yesno[3];
int ret;

	yesno[0] = yes_str; /* id = 1 */
	yesno[1] = no_str;  /* id = 2 */
	yesno[2] = NULL;
	ret = tboxf_choice(icb.input_screen,formats,text,args,yesno,NULL);
	return(ret == 1);
}
