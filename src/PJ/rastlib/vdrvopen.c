#define VDEV_INTERNALS
#include "vdevcall.h"

Errcode pj_open_ddriver(Vdevice **pvd, char *name)
/************************************************************************* 
 * Open up a loadable display driver. This is the call that PJ and the
 * driver torture program use as the first step in establishing 
 * a working graphics screen.  This call provides the driver with
 * more debugging functions than the standard pj_open_vdriver()
 * call.  Requires client to provide aa_syslib and aa_stdiolib.
 *
 * There should be a matching pj_close_vdriver() when the client is
 * finished with the driver.
 *
 * Parameters:
 *		Vdevice **pvd;			returns pointer to open video device
 *		char 	*name;			name of driver file (or pj_mcga_name for
 *								built in 320x200 MCGA/VGA driver)
 * Returns:
 *		Success (0)	if driver found
 *		negative error code on failure (see errcodes.h)
 *************************************************************************/
{
	if (!txtcmp(pj_get_path_name(name), pj_mcga_name))
		return(pj_open_mcga_vdriver(pvd));
	return(pj_open_loadable_vdriver(pvd,name));
}
