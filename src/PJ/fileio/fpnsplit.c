#include "errcodes.h"
#include "filepath.h"

static Errcode _fp_parse_suffix(char **pfn, char *suff)
{
char *fn = *pfn;

	if (*fn != '.')
		{
		suff[0] = 0;
		}
	else
		strncpy(suff,fn,5);
	return(Success);
}

static Errcode _fp_parse_file(char **pfn, char *file)
{
char *endfile;
char *fn = *pfn;
int len;

	endfile = pj_get_path_suffix(fn);
	len = endfile-fn;
	if (len > MAX_FILE_NAME_LEN)
		len = MAX_FILE_NAME_LEN;
	fn += sprintf(file,"%.*s", len, fn);
	if (strchr(file, '.') != NULL)
		return(Err_file_name_err);
	*pfn = fn;
}

Errcode fnsplit(char *path, char *device, char *dir, char *file,
	char *suffix)
/* split file name into smallest components */
{
Errcode err;
char upath[PATH_SIZE];
char *up = upath;

if (strlen(path) >= PATH_SIZE)
	return(Err_dir_too_long);
strcpy(upath, path);
strupr(upath);
if ((err  = _fp_parse_device(&up,device)) < Success)
	return(err);
if ((err = _fp_parse_dir(&up, dir)) < Success)
	return(err);
if ((err = _fp_parse_file(&up, file)) < Success)
	return(err);
return(_fp_parse_suffix(&up, suffix));
}


