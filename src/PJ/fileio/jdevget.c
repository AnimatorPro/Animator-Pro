#include "stdtypes.h"
#include "errcodes.h"
#include <ctype.h>

Errcode current_device(char *dstr)

/* gets current device name as string, returns string length of
 * device or error code does NOT include device delimitor after device name */
{
SHORT dev;

	if((dev = pj_dget_drive()) >= 0)
	{
		*dstr++ = dev + 'A';
		*dstr = 0;
		return(1);
	}
	return(pj_mserror(pj_dget_err()));
}
