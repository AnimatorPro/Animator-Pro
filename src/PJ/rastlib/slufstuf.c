#include "rastcall.h"

#ifdef SLUFFED
Coor clipdim(Coor *val,Coor length, Coor maxval)

/* clips *val and length to 0 and maxval returns new max
 * if max <= 0 clipped out */ 
{
register Coor max;

	if((max = *val) >= maxval)
		return(0);
	if(max < 0) 
		*val = 0;
	if((max += length) > maxval)
		return(maxval);
	return(max);
}
#endif /* SLUFFED */
#ifdef SLUFFED
Coor clipdim2(Coor *sval,Coor *dval,Coor length, Coor maxval)

/* given sdim, ddim, span, and max ddim returns the max ddim or ddim+length 
 * if <= 0 it is clipped out */
{
register Coor dmax;

	if((dmax = *dval) >= maxval)
		return(0);
	if(dmax < 0) 
	{
		*sval -= dmax;
		*dval = 0;
	}
	if((dmax += length) > maxval)
		return(maxval);
	return(dmax);
}
#endif /* SLUFFED */


