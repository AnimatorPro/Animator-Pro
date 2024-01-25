#define VDEV_INTERNALS
#include "vdevice.h"
#include "vdevinfo.h"

Errcode pj_vd_get_mode(Vdevice *vd, USHORT mode, Vmode_info *pvm)
/************************************************************************* 
 * Retrieve information about a mode from video driver.
 *
 * Parameters:
 *		Vdevice		*vd;		An open video driver.
 *		USHORT		mode;		Driver mode (less than vd->mode_count)
 *								we want information on.
 *		Vmode_info	*pvm;		Information on mode is stored here.
 * Returns:
 *		Success (0) if all is well, a negative error code on failure.
 *		See errcodes.h.
 * 
 *************************************************************************/
{
	return((*vd->lib->get_modes)(vd,mode,pvm));
}

int pj_vd_get_mode_count(Vdevice*vd)
/************************************************************************* 
 * Return the number of modes supported by driver.
 * 
 * Parameters:
 *		Vdevice		*vd;		An open video driver.
 *************************************************************************/
{
	return(vd->mode_count);
}
