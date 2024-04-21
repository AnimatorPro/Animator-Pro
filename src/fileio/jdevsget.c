#include "jfile.h"
#include "msfile.h"

int pj_get_devices(UBYTE *devices)

/* Get list of devices we believe to be real (for drive buttons on
   browse menu and other uses) by doing a request for info DOS call
   on each letter of the alphabet.  Since this is a little slow on 
   floppies, we consult the BIOS equipment list for a count of # of
   floppies to fill in the potential A: and B: devices. 
   Returns count of devices.  If devices array pointer is NULL 
   the only the count is returned. This is slow on the first call but
   subsequent calls are negligible */
{
static ULONG devmask = 0;  /* one bit on for each valid device */
int i, floppies, dev_count;


	dev_count = 0;

	if(!devmask)
	{
		floppies = pj_dcount_floppies();
		for(i=0; i < 26; ++i)
		{
			if(i < 2)
			{
				if(i >= floppies)
					continue;
			}
			else if(pj_dis_drive(i) < Success)
				continue;
			devmask |= (1<<i);
		}
	}

	for(i=0;i < 26;++i)
	{
		if(devmask & (1<<i))
		{
			if(devices)
				*devices++ = i;
			++dev_count;
		}
	}
	return(dev_count);
}
