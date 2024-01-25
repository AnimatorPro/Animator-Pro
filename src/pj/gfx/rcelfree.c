#include "cmap.h"
#include "memory.h"
#include "rcel.h"

void pj_rcel_close(Rcel *rc)
{
	if(rc==NULL)
		return;
	pj_cmap_free(rc->cmap);
	rc->cmap = NULL;
	pj_close_raster(rc);
}
void pj_rcel_free(Rcel *c)
/************************************************************************* 
 * Free all resources associated with an Rcel (a colormap/raster 
 * combination).
 *
 * Parameters:
 *		Rcel *c;		The Rcel to dispose of.
 *************************************************************************/
{
	if(!c)
		return;
	pj_rcel_close(c);
	pj_free(c);
}
