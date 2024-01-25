#include "stfont.h"
#include "fontdev.h"
#include "rastext.h"

Vfont *get_sys_font(void)
{
static char init = false;
static Vfont sysfont;

	if(!init)
	{
		init_sixhi_vfont(&sysfont);
		init = true;
	}
	return(&sysfont);
}
