#include "dfile.ih"

Boolean pj_exists(char *path)
/* Does file exist? Boolean does not handle.j errors now */
{
Jfile f;

	if ((f = pj_open(path, JREADONLY)) == JNONE)
		return(FALSE);
	pj_close(f);
	return(TRUE);
}
