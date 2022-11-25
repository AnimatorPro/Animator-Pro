#include "jimk.h"

Errcode cant_use_module(Errcode err, char *modname)
{
	return(softerr(err,"!%s", "mod_unable", modname));
}
