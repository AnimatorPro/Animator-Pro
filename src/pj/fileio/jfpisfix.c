#include "jfile.h"

Errcode pj_pathdev_is_fixed(char *path)
/* returns 1 if device is fixed 0 if not < 0 if error */
{
char device[DEV_NAME_LEN];
Errcode err;

	if((err = get_path_device(path,device)) < Success)
		return(err);
	return(pj_is_fixed(device));
}
