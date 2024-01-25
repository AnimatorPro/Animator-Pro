#include "player.h"
#include "ftextf.h"

#ifdef SLUFFED
Errcode script_error(Errcode err,char *text,...)
{
char buf[256];
char *formats;
va_list args;

	if(err >= Success || err == Err_reported || err == Err_abort)
		return(err);

	buf[0] = 0;
	if(text)
	{
		buf[0] = '\n';
		va_start(args,text);
		formats = ftext_format_type(&text,&args);
		vnsftextf(&buf[1],sizeof(buf)-1,formats,text,args);
		va_end(args);
	}

	return(softerr(err,"!%s%d%s", "play_script_err", 
				   pcb.cn->scr_path, pcb.cn->line+1, buf));
}
#endif /* SLUFFED */

Errcode soft_script_error(Errcode err,char *key,...)
{
char buf[256];
char text[256];
char *formats;
va_list args;

	if(err >= Success || err == Err_reported || err == Err_abort)
		return(err);

	buf[0] = 0;
	if(key)
	{
		va_start(args,key);
		formats = ftext_format_type(&key,&args);
		soft_name_string("play_serrors", key, text, sizeof(text));
		buf[0] = '\n';
		vnsftextf(&buf[1],sizeof(buf)-1,formats,text,args);
		va_end(args);
	}
	return(softerr(err,"!%s%d%s", "play_script_err", 
				   pcb.cn->scr_path, pcb.cn->line+1, buf));
}
