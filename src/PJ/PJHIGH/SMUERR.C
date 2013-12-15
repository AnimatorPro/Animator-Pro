
#include "ftextf.h"
#include "commonst.h"
#include "softmenu.h"
#include "errcodes.h"

static Errcode soft_errline(Errcode err,char *errsym,char *key,va_list *pargs)
/* note, will not report if Successful or Err_aborted or Err_reported */
{
char etext[ERRTEXT_SIZE];
char *formats;
char text[256];		/* buffer for specific text, only used if key != NULL */

	if(!get_errtext(err,etext))
		return(err);

	if(!key)
	{
		text[0] = 0;
		formats = NULL;
	}
	else
	{
		formats = ftext_format_type(&key, pargs);
		soft_name_string(errsym, key, text, sizeof(text));
	}
	varg_continu_box(formats,text,(*pargs),etext);
	return(Err_reported);
}


Errcode softerr(Errcode err,char *key,...)
{
va_list args;

va_start(args,key);
err = soft_errline(err,"errlines",key,&args);
va_end(args);
return(err);
}


