#include "rastcall.ih"
#include "errcodes.h"
#include "libdummy.h"

Errcode pj_open_nullrast(Raster *r) /* opens a "null" raster that dumps
									* input and performs no action */
{
	if(!r->width || !r->height)
		return(Err_bad_input);
	r->lib = pj_get_null_lib();
	r->type = RT_NULL;
	return Success;
}

