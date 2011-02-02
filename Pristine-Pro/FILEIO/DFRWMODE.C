#include "dfile.ih"

int get_jmode(Jfl *t)
{
	if(t)
		return(t->rwmode);
	return(JUNDEFINED);
}
