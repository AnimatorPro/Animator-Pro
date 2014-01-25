#include <ctype.h>
#include "errcodes.h"
#include "filepath.h"
#include "msfile.h"

Errcode pj_change_device(char *name)

/* Hey dos - I want to go to this device. Accepts device name with or without
 * delimitor e: "A" ir "A:" */
{
int d;

	if(!name)
		return(Err_null_ref);
	if(!name[0] || (name[1] && ((name[1] != DEV_DELIM) || name[2])))
		return(Err_no_device);

	d = name[0];
	if(islower(d))
		d -= 'a';
	else
		d -= 'A';

	/* 0 == A: 1 == b: */

	if(d < 2  /* attempt to go to "a:" or "b:" drive */
	   && d >= pj_dcount_floppies())
	{
		return(Err_no_device);
	}
	return(pj_dset_drive(d));
}
