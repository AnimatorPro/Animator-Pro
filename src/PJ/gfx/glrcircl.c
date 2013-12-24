#include "gfx.h"

Errcode rcircle(SHORT xcen, SHORT ycen, SHORT rad, 
	VFUNC dotout, void *dotdat, EFUNC hlineout, void *hlinedat, Boolean filled)

/* radius circle */
{
	return(dcircle(xcen,ycen,(rad<<1)+1,dotout,dotdat,
				   hlineout,hlinedat,filled));
}
