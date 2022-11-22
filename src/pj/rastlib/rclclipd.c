#include "rastcall.h"

Coor pj_lclipdim(Coor *val,Coor length, Coor maxval)
/* same as clipdim but returns new length instead of new max */
{
Coor dif;

	if((dif = *val) < 0)
	{
		length += dif;
		*val = dif = 0;
	}
	if((dif += (length - maxval)) > 0)
		length -= dif;
	return(length);
}
