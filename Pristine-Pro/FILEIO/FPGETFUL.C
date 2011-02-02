#include "errcodes.h"
#include "filepath.h"
#include "jfile.h"
#include "tfile.h"

int _fp_get_path_devlen(char *path)

/* returns device name length including delimiter of path Error if invalid
 * 0 if no device found */
{
char upperc;

	if(*path && (path[1] == DEV_DELIM))
	{
		switch(*path)
		{
			case TDEV_MED:
			case TDEV_LO:
			case TRD_CHAR:
				break;
			default:
				upperc = toupper(*path);
				if(upperc < 'A' || upperc > 'Z')
					return(Err_no_device);
				break;
		}
		return(2);
	}
	return(0);
}
Errcode get_full_path(char *path, char *fullpath)

/* expands a path to the fully expanded path from device down for the path */
{
Errcode err;
char pbuf[PATH_SIZE];
int len;
char devnum;


	if(path == fullpath) /* if dest is source */
	{
		strcpy(pbuf,path);
		path = pbuf;
	}

	if((len = _fp_get_path_devlen(path)) != 0)
	{
		if(len < 0)
			return(len);

		switch(*path)
		{
			case TDEV_MED:
			case TDEV_LO:
			case TRD_CHAR:
				strcpy(fullpath, path);
				return(Success);
		}
		strncpy(fullpath,path,len);
		path += len;
	}
	else
	{
		if((len = current_device(fullpath)) < 0)
			return(len);
		fullpath[len] = DEV_DELIM; /* install device delimitor */
		++len;
	}

	/* ms dos has letters for devices */
	fullpath[0] = toupper(fullpath[0]);
	devnum = fullpath[0] - 'A'; /* ms dos devices */

	fullpath += len;

	if(*path == DIR_DELIM) /* ms dos land */
		goto done;

	*fullpath++ = DIR_DELIM;
	if((err = pj_dget_dir(1 + devnum, fullpath)) != Success)
		return(pj_mserror(err));

	if((len = strlen(fullpath)) > 0)
	{
		fullpath += len;
		*fullpath++ = DIR_DELIM;
	}
	if(len + strlen(path) >= PATH_SIZE)
		return(Err_dir_too_long);

done:
	strcpy(fullpath,path);
	return(Success);
}
