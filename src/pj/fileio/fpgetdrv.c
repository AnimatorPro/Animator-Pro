#include <ctype.h>
#include "errcodes.h"
#include "filepath.h"
#include "msfile.h"
#include "tfile.h"

Errcode get_path_device(char *path,char *device)
/* for ms dos: returns device for a path < 0 if error */
{
char drive;

	if(path[0] != 0 && path[1] == ':')
	{
		if(!is_tdrive(path))
		{
			drive = toupper(path[0]);
			if(drive < 'A' || drive > 'Z')
			{
				*device = 0;
				return(Err_no_device);
			}
		}
		*device++ = *path++;
		*device = 0;
		return(Success);
	}
	return(current_device(device));
}
