#include "cmap.h"

LONG _h_lsrgb_value(LONG n1, LONG n2, SHORT hue)
{
LONG val;

	while (hue >= 256)
		hue -= 256;

	while (hue < 0)
		hue += 256;

	if (hue < (256/6))
		val = n1 + (n2 - n1) * (long)hue / (256/6);
	else if (hue < 256/2)
		val = n2;
	else if (hue < 2*256/3)
		val = n1 + (long)(n2 - n1) * (2*256/3 - hue) / (256/6);
	else
		val = n1;

	val>>=8;
	if (val > 255) val = 255;
	if (val<0) val = 0;
	return(val);
}
