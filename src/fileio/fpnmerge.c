#ifdef SLUFFED
#include "errcodes.h"
#include "filepath.h"

Errcode fnmerge(char *path, char *device, char *dir, char *file,
	char *suffix)
{
if (strlen(device)+strlen(dir)+strlen(file)+strlen(suffix) > PATH_SIZE)
	return(Err_dir_too_long);
strcpy(path,device);
strcat(path,dir);
strcat(path,file);
strcat(path,suffix);
return(Success);
}
#endif /* SLUFFED */
