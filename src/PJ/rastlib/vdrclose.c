#define VDEV_INTERNALS
#include "memory.h"
#include "errcodes.h"
#include "stdtypes.h"
#include "vdevcall.h"
#include "memory.h"

extern void *pj_get_grc_lib(void);

/* next rtype for type assigning */
static SHORT next_rtype = RT_FIRST_VDRIVER;

void pj_close_vdriver(Vdevice **pvd)
/************************************************************************* 
 * This is the inverse of pj_open_ddriver() or pj_open_vdriver().
 * Call this to when you are finished with a display driver to free
 * up all resources associated with the driver.
 *
 * Parameters:
 *		Vdevice **pvd;	 Vdevice from pj_open_ddriver() or pj_open_vdriver()
 *************************************************************************/
{
Vdevice *vd = *pvd;
typedef void (*Vd_cleanup)(Vdevice *vd);

	if(vd == NULL)
		return;

	if((vd->first_rtype + vd->num_rtypes) == next_rtype)
		next_rtype = vd->first_rtype;

	if(vd->lib != NULL)
	{
		if (vd->lib->close_graphics != NULL)
			(*vd->lib->close_graphics)(vd);
	}
	if(vd->hdr.host_data != NULL) /* this is a loaded driver */
		pj_rexlib_free((Rexlib **)pvd);
	else if(vd->hdr.cleanup)  /* one of our static drivers */
		((Vd_cleanup)(vd->hdr.cleanup))(vd);
	*pvd = NULL; 
}
Errcode pj__vdr_initload_open(Errcode (*loadit)(Vdevice **pvd,char *name),
					       Vdevice **pvd, char *name)

/* subroutine that calls a routine (*loadit)() that provides the (loaded)
 * library */ 
{
Vdevice *vd;
Errcode err;

	if ((err = (*loadit)(pvd,name)) < Success)
		goto error;
	vd = *pvd;

	if((err = pj_rexlib_init(&vd->hdr)) < Success)
		goto error;

	if(vd->num_rtypes == 0)
	{
		err = Err_driver_protocol;
		goto error;
	}
	vd->first_rtype = next_rtype;
	vd->grclib = pj_get_grc_lib();
	next_rtype += vd->num_rtypes; /* only done if load is successful */

	goto done;
error:
	pj_close_vdriver(pvd);
done:
	return(err);
}
