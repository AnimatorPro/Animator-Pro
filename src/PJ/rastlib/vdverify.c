#define VDEV_INTERNALS
#include "errcodes.h"
#include "vdevice.h"

Errcode pj_vd_verify_hardware(Vdevice *vd)
/* verifys driver hardware exists */
{
	if (vd->lib->detect == NULL)
		return(Err_driver_protocol);
	return((*vd->lib->detect)(vd));
}
