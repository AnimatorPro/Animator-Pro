#include "rcel.h"
#include "memory.h"
#include "errcodes.h"

Errcode pj_alloc_mcga_rcel(Rcel **pcel)

/* allocates an rcel from vdriver for specs given */
{
Rcel *cel;
Errcode err;

	if((cel = pj_zalloc(sizeof(Rcel))) == NULL)
	{
		err = Err_no_memory;
		goto error;
	}
	if((err = pj_open_mcga_rcel(cel)) < Success)
		goto error;
	goto done;

error:
	pj_freez(&cel);
done:
	*pcel = cel;
	return(err);
}
