#include "stfont.h"
#include "fontdev.h"

Vfont *get_sys_font()
{
static char init = FALSE;
static Vfont sysfont;

	if(!init)
	{
		init_sixhi_vfont(&sysfont);
		init = TRUE;
	}
	return(&sysfont);
}
