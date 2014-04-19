#include <ctype.h>
#include <string.h>
#include "errcodes.h"
#include "jfile.h"
#include "msfile.h"

/* Function: change_dir
 *
 *  Hey dos - I want to go to this directory.  Actually this changes
 *  both device and directory at once.  e.g. name could be
 *  C:\VPAINT\FISHIES or C:\VPAINT\FISHIES\
 */
Errcode
change_dir(const char *name)
{
int odevice = -1;
Errcode err;
char *lastbyte;
char lastval;
int namelen;
struct devname { char name[3]; } device;

	if(!name || !name[0])
		return(Success);

	if(name[1] == ':')	/* got a device... */
	{
		odevice = pj_dget_drive();

		/* Move device name to temp buffer and null terminate */
		device = *((struct devname *)name);
		device.name[2] = 0;
		if((err = pj_change_device(device.name)) < Success)
			return(err);
		name += 2;
	}
	if (!name[0])
		return(Success);

	namelen = strlen(name) - 1;
	lastbyte = name + namelen;

	/* _lodos_set_dir() can't have trailing '\' but must have it for root */

	if ((lastval = *lastbyte) == DIR_DELIM && namelen > 0)
		*lastbyte = 0;

	err = pj_mserror(_lodos_set_dir(name));
	*lastbyte = lastval;

	if(err < Success)
	{
		if(odevice >= 0)
			pj_dset_drive(odevice);
		return(err);
	}
	return(Success);
}
