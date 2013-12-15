#include "errcodes.h"
#include "filepath.h"

Errcode _fp_parse_dir(char **pfn, char *dir)
{
char *fn = *pfn;
int dirlen;

	if((dirlen = get_drawer_len(fn)) > MAX_DIR_LEN)
		return(Err_dir_too_long);
	fn += sprintf(dir,"%.*s",dirlen,fn);
	if (strchr(dir, ':'))	/* make sure no colons */
		return(Err_dir_name_err);
	*pfn = fn;
	return(dirlen);
}
