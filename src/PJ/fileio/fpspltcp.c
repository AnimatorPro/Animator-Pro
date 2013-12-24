#include "errcodes.h"
#include "filepath.h"

Errcode split_copy_path(char *file_name, char *path_side, char *file_side)
/* split file path into file name and path sides */
{
Errcode err;

strupr(file_name);
if ((err = _fp_parse_device(&file_name, path_side)) < Success)
	return(err);
path_side += err;
if ((err = _fp_parse_dir(&file_name, path_side)) < Success)
	return(err);
strcpy(file_side, file_name);
return(Success);
}
