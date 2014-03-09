#include "ftextf.h"
#include "reqlib.h"
#include "softmenu.h"

Boolean
vsoft_qreq_number(short *inum, short min, short max, char *key, va_list args,
		Errcode (*update)(void *data, SHORT val), void *uddat)

/* subroutine to various soft_qreq_number routines */
{
Boolean ret;
char *text;
char *formats;

 	if(soft_load_ftext_type(key,&args,&formats,&text) < Success)
		ret = FALSE;
	else
		ret = varg_qreq_number(inum,min,max,update,uddat,formats,text,args);

	smu_free_text(&text);
	return(ret);
}

