#include <stdarg.h>
#include "errcodes.h"
#include "pjbasics.h"
#include "reqlib.h"
#include "ftextf.h"
#include "softmenu.h"

/* 80 bytes of static buffer, I assume this saves more string 
 * space than that. Keeping it around eliminates the kerchunk */

static char soft_key[32];
static char soft_text[TTEXTF_MAXCHARS];

Errcode soft_ttextf(char *key,va_list *pargs)
{
char *formats;

	formats = ftext_format_type(&key,pargs);
	if(strncmp(soft_key,key,sizeof(soft_key)) != 0)
	{
		strncpy(soft_key,key,sizeof(soft_key));
		soft_string(key,soft_text,sizeof(soft_text));
	}
	return(ttextf(soft_text,*pargs,formats));
}
