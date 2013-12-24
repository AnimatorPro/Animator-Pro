#include "rcel.h"
#include "memory.h"
#include "errcodes.h"
#include "vdevcall.h"

Errcode pj_rcel_custom_alloc(Rasthdr *spec, Rcel **pcel,
							 Grctype grc_lib_type, LONG num_colors)

/* allocates an rcel from vdriver for specs given */
{
Rcel *cel;
Errcode err;

	if((cel = pj_zalloc(sizeof(Rcel))) == NULL)
	{
		err = Err_no_memory;
		goto error;
	}
	if((err = pj_rcel_custom_open(spec,cel,grc_lib_type,num_colors)) < Success)
		goto error;
	goto done;

error:
	pj_freez(&cel);
done:
	*pcel = cel;
	return(err);
}
