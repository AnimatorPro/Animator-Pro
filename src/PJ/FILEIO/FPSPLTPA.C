#include "errcodes.h"
#include "filepath.h"

Errcode split_path(char *p1, char *device, char *dir)
/* split a device:dir combination into device and dir */
{
char file[12];
char suffix[12];
char bbuff[16];
Errcode err;

strupr(p1);
if ((err = fnsplit(p1, device,dir,file,suffix)) < Success)
	return(err);
if (device[0] == 0)
	return(Err_no_device);
if (suffix[0])
	sprintf(bbuff, "%s%s", file, suffix);
else
	strcpy(bbuff, file);
if (bbuff[0])
	{
	strcat(bbuff, "\\");
	if (strlen(dir)+strlen(bbuff) >= PATH_SIZE-2)	/* -2 for D: */
		return(Err_dir_too_long);
	strcat(dir,bbuff);
	}
return(Success);
}
