#include "dfile.ih"

Errcode pj_delete(char *name)
{
Errcode err;
	if ((err = pj_ddelete(name)) < Success)
		jerr = err;
	return(err);
}
