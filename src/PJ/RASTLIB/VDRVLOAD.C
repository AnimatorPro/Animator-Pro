#define VDEV_INTERNALS
#include "rastlib.h"
#include "vdevcall.h"
#include "rexlib.h"


static Errcode load_driver(Vdevice **pvd,char *name)
{
static Libhead *libs_for_vdrivers[] = { &aa_syslib, &aa_stdiolib, NULL };

	return(pj_rexlib_load(name, REX_VDRIVER, 
		    (Rexlib **)pvd,libs_for_vdrivers,NULL));
}
Errcode pj_open_loadable_vdriver(Vdevice **pvd, char *name)
{
	return(pj__vdr_initload_open(load_driver,pvd,name));
}
