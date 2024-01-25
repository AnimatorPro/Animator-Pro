
#include "stdtypes.h"
#include "errcodes.h"

Errcode pj_dset_drive(int d)
{
Errcode err;

	if((err = pj_mserror(lo_dos_set_drive(d))) != Success)
	{
		if (d <= 1)	/* Phar lap has troubles with floppies... */
		{
			if(pj_dget_drive() == d)
				err = Success;
		}
	}
	return(err);
}
