#include "pjbasics.h"

Errcode cant_load(Errcode err, char *name)
{
	return(softerr(err,"!%s","cant_load",name));
}
