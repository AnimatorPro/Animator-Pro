#include "errcodes.h"
#include "raster.h"
#include "memory.h"

Errcode pj_alloc_bytemap(Rasthdr *spec, Bytemap **prr)

/* this will allocate and initialize an entire Bytemap using the input Bytemap
 * as a specification for what size etc to open up. It must be supplied with
 * init or it will crash!! a typical call would be.
 *
 *		if(pj_alloc_bytemap(spec,&rr) < 0)
 *			goto error;
 *
 */
{
register Bytemap *rr;
Errcode err;

	if((rr = pj_malloc((LONG)sizeof(Bytemap))) == NULL)
	{
		err = Err_no_memory;
		goto error;
	}
	if((err = pj_open_bytemap(spec,rr)) < 0)
		goto error;

	*prr = rr;
	return(0);
error:
	pj_gentle_free(rr);
	return(err);
}
