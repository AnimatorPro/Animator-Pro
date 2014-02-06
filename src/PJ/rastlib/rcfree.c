#include "memory.h"
#include "rastcall.ih"
#include "memory.h"

void pj_rast_free(Raster *rp)

/* this frees a raster that has been allocated and opened with a call such 
 * as alloc_ramrast() the library should return an error if it was not
 * opened by this library's driver  */
{
	if(rp == NULL)
		return;

	if(CLOSE_RAST(rp) >= Success)
		pj_free(rp);
}
