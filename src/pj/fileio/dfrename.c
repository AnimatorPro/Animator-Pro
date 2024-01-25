#include "dfile.ih"

Errcode pj_rename(char *old, char *new)
{
	return(pj_mserror(pj_drename(old,new)));
}
