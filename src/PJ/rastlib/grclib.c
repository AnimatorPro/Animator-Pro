/* grcfulib.c - Contains all remaining calls not contained in the
   comp calls dcomp calls or common calls and a function to load
   all of them into a fully loaded library
   
   	   Generic display driver for hi-res animator.
   Useful to fill in parts of a driver that are not implemented.
   Requires you to fill in _get_dot and _put_dot.  The rest of
   the system will funnel through these.  Over-ride other
   functions for increased performance.  Does reasonably well 
   speedwise if _get_hseg(), _put_hseg(), and pj__set_hline() are 
   implemented as higher level functions such as the blits go through
   these. */

#include "libdummy.h"
#include "rastcall.h"
#include "rastlib.h"

void *pj_get_grc_lib(void)
{
static Boolean loaded = FALSE;
static Rastlib grc_lib;

	if(!loaded)
	{
		pj_init_null_rastlib(&grc_lib);
		pj_grc_load_fullcalls((struct rastlib *)&grc_lib);
		loaded = 1;
	}
	return(&grc_lib);
}
