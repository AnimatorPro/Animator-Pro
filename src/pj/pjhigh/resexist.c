#include "jfile.h"
#include "resource.h"

bool resource_exists(char *name)
{
char rname[PATH_SIZE];
	return(pj_exists(make_resource_name(name,rname)));
}
