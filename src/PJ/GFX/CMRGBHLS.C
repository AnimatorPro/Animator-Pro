
/* cmrgbhls.c - Routines to convert between rgb and hls representation
   of colors.  RGB coordinates are assumed 0 - 255.  HLS
   coordinates are 0-255. */

#include "imath.h"

/****
These rgb to hls routines work with rgbhls all in [0,255]
****/

extern LONG _h_lsrgb_value(LONG n1, LONG n2, SHORT hue);
#define value(n1,n2,hue) _h_lsrgb_value(n1,n2,hue)


void rgb_to_hls(SHORT r,SHORT g,SHORT b,SHORT *h,SHORT *l,SHORT *s)

/*routine copped from p618 of Foley and Van Dam Fundamentals of Interactive
  computer graphics.  Converted to integer math by Jim Kent.*/
{
SHORT max,min;
SHORT rc,gc,bc;
SHORT hv, lv, sv;

max = r;
if (g>max) max = g;
if (b>max) max = b;
min = r;
if (g<min) min = g;
if (b<min) min = b;
lv = (max+min)>>1;

if (max == min)
	{
	hv = sv = 0;
	}
else
	{
	if (lv < 128)
		sv = ((long)(max-min)<<8)/(max+min);
	else
		sv = ((long)(max-min)<<8)/(512-max-min);
	
	if (sv >= 256)
		sv = 255;
	rc = pj_uscale_by(256, max-r, max-min);
	gc = pj_uscale_by(256, max-g, max-min);
	bc = pj_uscale_by(256, max-b, max-min);

	if (r == max)
		hv = bc - gc;
	else if (g == max)
		hv = 2*256 + rc - bc;
	else
		hv = 4*256 + gc - rc;
	
	hv /= 6;
	while (hv < 0) hv += 256;
	while (hv >= 256) hv -= 256;
	}
*h = hv;
*l = lv;
*s = sv;
}
