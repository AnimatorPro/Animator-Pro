#include <ctype.h>
#include <string.h>
#include "errcodes.h"
#include "filepath.h"
#include "jfile.h"
#include "msfile.h"
#include "tfile.h"

/* Function: _fp_get_path_devlen
 *
 *  Returns the device name length including the delimiter.
 *  Returns Error_no_device if invalid, 0 if no device found.
 */
int
_fp_get_path_devlen(const char *path)
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
