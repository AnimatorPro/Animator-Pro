#include "rastcall.ih"
#include "errcodes.h"
#include "memory.h"

Errcode pj_alloc_bitmap(Rasthdr *spec, Bitmap **prr)

/* this will allocate and initialize an entire Bitmap using the input Bitmap
 * as a specification for what size etc to open up. It must be supplied with
 * init or it will crash!! a typical call would be.
 *
 * Bitmap rrinit;
 * Bitmap *rr;
 *
 *		init_bitmap(NULL,&rrinit)
 *		load rrinit specifications here;
 *		if(pj_alloc_bitmap(&rrinit,&rr) < 0)
 *			goto error;
 *
 */
{
register Bitmap *rr;
Errcode err;

	if((rr = pj_malloc((LONG)sizeof(Bitmap))) == NULL)
	{
		err = Err_no_memory;
		goto error;
	}
	if((err = pj_open_bitmap(spec,rr)) < 0)
		goto error;

	*prr = rr;
	return(0);
error:
	pj_gentle_free(rr);
	return(err);
}
