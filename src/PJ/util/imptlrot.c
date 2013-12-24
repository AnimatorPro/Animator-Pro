#include "imath.h"

void partial_rot(SHORT theta, SHORT *xx, SHORT *yy)
/* 2 integer math dimensional rotation */
{
SHORT s, c;
SHORT x, y;

	x = *xx;
	y = *yy;

	s = isin(theta);
	c = icos(theta);

	*xx = itmult(x,c) + itmult(y,s);
	*yy = itmult(y,c) + itmult(x,-s);
}
