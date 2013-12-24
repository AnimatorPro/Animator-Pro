#include "rastcall.h"

Coor pj_lclipdim2(Coor *sval,Coor *dval,Coor length, Coor maxval)
/* same as clipdim2 but returns new length instead of new max */
{
Coor dif;

	if((dif = *dval) < 0)
	{
		*dval = 0;
		*sval -= dif;
		length += dif;
	}
	if((dif = *dval + length - maxval) > 0)
		length -= dif;
	return(length);
}
