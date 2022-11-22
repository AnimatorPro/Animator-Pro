#include "memory.h"
#include "errcodes.h"
#include "rcel.h"
#include "vdevcall.h"

Errcode alloc_display_rcel(Vdevice *vd, Rcel **pcel,
						   USHORT width, USHORT height, SHORT mode)
/************************************************************************* 
 * This function creates a visible raster cel which we can draw on or play 
 * a fli on from an open display driver.  The cel consists of two parts:
 * the pixels and the color map.
 *
 * There should be a matching pj_rcel_free(*pcel) to this call.
 *
 * Parameters:
 *		Vdevice		*vd;		An open display driver.
 *		Rcel		**pcel;		Used to return cel.
 *		USHORT		width;		Desired cel width.  Must be less than
 *								or equal to the highest resolution
 *								supported by driver in mode.
 *		USHORT		height;		Desired cel height.  Likewise must be
 *								less than or equal to mode's highest res.
 *		SHORT		mode;		Driver display mode.
 *************************************************************************/
{
Rcel *cel;
Errcode err;

	if((cel = pj_zalloc(sizeof(Rcel))) == NULL)
	{
		err = Err_no_memory;
		goto error;
	}
	if((err = open_display_rcel(vd,cel,width,height,mode)) < Success)
		goto error;
	goto done;

error:
	pj_freez(&cel);
done:
	*pcel = cel;
	return(err);
}
