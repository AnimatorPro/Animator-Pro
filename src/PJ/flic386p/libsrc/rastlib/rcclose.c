#include "rastcall.ih"

void pj_close_raster(Raster *rp)
{
	if(rp != NULL && rp->lib != NULL && rp->lib->close_raster != NULL)
		CLOSE_RAST(rp);
}
