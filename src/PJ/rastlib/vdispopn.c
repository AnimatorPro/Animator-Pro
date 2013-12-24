#define VDEV_INTERNALS
#include "rastlib.h"
#include "vdevcall.h"
#include "rexlib.h"


static Errcode load_driver(Vdevice **pvd,char *name)
{
static Libhead *libs_for_vdrivers[] = { &aa_syslib, NULL };

	return(pj_rexlib_load(name, REX_VDRIVER, 
		    (Rexlib **)pvd,libs_for_vdrivers,NULL));
}

Errcode pj_open_vdriver(Vdevice **pvd, char *name)
/************************************************************************* 
 * Open up a loadable display driver. This is the call that most 
 * display driver clients will use as the first step in establishing 
 * a working graphics screen.  Clients wishing to provide the driver
 * with extra debugging functions use pj_open_ddriver() instead.  This
 * requires the client provide the library aa_syslib.
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
	return(pj__vdr_initload_open(load_driver,pvd,name));
}
