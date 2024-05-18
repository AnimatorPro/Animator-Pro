#include "rcel.h"
#include "memory.h"
#include "errcodes.h"
#include "vdevcall.h"


Errcode alloc_vd_rcel(Vdevice *vd, Rasthdr *spec, Rcel **pcel,
					  LONG num_colors, UBYTE displayable)

/* allocates an rcel from vdriver for specs given */
{
Rcel *cel;
Errcode err;

	if((cel = pj_zalloc(sizeof(Rcel))) == NULL)
	{
		err = Err_no_memory;
		goto error;
	}
	if((err = open_vd_rcel(vd,spec,cel,num_colors,displayable)) < Success)
		goto error;
	goto done;

error:
	pj_freez(&cel);
done:
	*pcel = cel;
	return(err);
}
