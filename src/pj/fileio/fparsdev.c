#include <stdio.h>
#include "errcodes.h"
#include "filepath.h"

int _fp_parse_device(char **pfn, char *device)
{
char *fn = *pfn;
int len;

	if((len = _fp_get_path_devlen(fn)) >= Success)
		*pfn += sprintf(device,"%.*s", len, fn);
	return(len);
}
