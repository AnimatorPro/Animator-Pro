
/* cmhlsrgb.c - Routines to convert between rgb and hls representation
   of colors.  RGB coordinates are assumed 0 - 255.  HLS
   coordinates are 0-255. */

#include "cmap.h"

/****
These rgb to hls routines work with rgbhls all in [0,255]
****/

#define value(n1,n2,hue) _h_lsrgb_value(n1,n2,hue)

void hls_to_rgb(SHORT *r, SHORT *g, SHORT *b, SHORT h, SHORT l, SHORT s)
{
long	m1, m2;
SHORT rv, gv, bv;

if (l <= 128)
	m2 = (long)l * (256 + s);
else
	m2 = ((long)(l + s)<<8) - (long)l * s;

m1 = 512 *(long)l - m2;

if (s == 0)
	{
	rv = l;
	gv = l;
	bv = l;
	}
else
	{
	s = value(m1, m2, h - 256/3);
	l = value(m1, m2, h);
	h = value(m1, m2, h + 256/3);
	bv = s;
	gv = l;
	rv = h;
	}
/* scale down to VGA values */
rv += 2;
bv += 2;
gv += 2;
if (rv > 255)
	rv = 255;
if (gv > 255)
	gv = 255;
if (bv > 255)
	bv = 255;
*r = rv;
*g = gv;
*b = bv;
}


