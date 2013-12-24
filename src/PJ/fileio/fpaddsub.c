#include "errcodes.h"
#include "filepath.h"

Errcode add_subpath(char *drawer, char *subpath, char *outpath)
/* puts path and name together to for a concatenated path, record will be
 * truncated to PATH_SIZE outpath may be a drawer input drawer and output 
 * path may be the same buffer */
{
int len;
char *opstart = outpath;


	if(drawer == outpath)
		len = strlen(drawer);
	else
		len = sprintf( outpath, "%.*s", PATH_SIZE-1, drawer);

	if(len >= PATH_SIZE-1)
		return(Err_dir_too_long);

	if(len) /* if an upper level path */
	{
		/* make sure there is a trailing '\' */

		outpath += len;
		if(outpath[-1] != DIR_DELIM)
		{
			++len;
			*outpath++ = DIR_DELIM;
		}
	}

	if(len + strlen(subpath) >= PATH_SIZE)
		return(Err_dir_too_long);

	sprintf( outpath, "%.*s", (PATH_SIZE-1)-len , subpath);
	return(Success);
}
