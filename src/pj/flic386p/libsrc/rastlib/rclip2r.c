#include "rastcall.h"

Coor pj_lclip2rects(Coor *aval, Coor *bval,
				 Coor length, Coor amax, Coor bmax)

/* clips a dimention to two maximums this assumes both mins are 0 */
{
Coor dif;

	if(*aval< *bval) /* biggest difference wins */
		dif = *aval;
	else
		dif = *bval;

	if(dif < 0)
	{
		length += dif;
		*aval-= dif;
		*bval-= dif;
	}
	if((dif = *aval+ length - amax) > 0)
		length -= dif;
	if((dif = *bval+ length - bmax) > 0)
		length -= dif;
	return(length);
}
