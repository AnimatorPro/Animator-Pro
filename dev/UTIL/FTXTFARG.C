#include "ftextf.h"

Errcode init_ftextfarg(Ftextfarg *fa, char *formats, char *text)
{
	if(!formats)
		formats = "";
	return(init_eitherfarg(fa,formats,text));
}
