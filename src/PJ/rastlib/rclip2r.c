#include "rastcall.h"

Ucoor pj_lclip2rects(Coor *aval, Coor *bval,
		Ucoor orig_length, Ucoor amax, Ucoor bmax)
{
Coor length = orig_length;
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

	return (length >= 0) ? length : 0;
}
