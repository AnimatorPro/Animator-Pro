#ifdef SLUFFED
#include "gfx.h"

Errcode rcircle(SHORT xcen, SHORT ycen, SHORT rad, 
		dotout_func dotout, void *dotdat,
		hline_func hlineout, void *hlinedat,
		Boolean filled)
/* radius circle */
{
	return(dcircle(xcen,ycen,(rad<<1)+1,dotout,dotdat,
				   hlineout,hlinedat,filled));
}
#endif /* SLUFFED */
