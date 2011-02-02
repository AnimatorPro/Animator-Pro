/*****************************************************************************
 * rgbhls.c - Routines to convert between rgb and hls representation.
 *
 * RGB coordinates are assumed 0 - 255.  HLS coordinates are 0-255.
 *
 * (Note: this code cobbled together from various PJ source modules for
 * use in colorutl poe.)
 ****************************************************************************/

#include "stdtypes.h"

static int pj_uscale_by(USHORT x, USHORT p, USHORT q)
/*****************************************************************************
 * return(x * p/q) done to avoid rounding error
 ****************************************************************************/
{
LONG l;

	l = x;
	l *= p;
	l /= q;
	return((int)l);
}

static LONG hlsrgb_value(LONG n1, LONG n2, SHORT hue)
/*****************************************************************************
 *
 ****************************************************************************/
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


void hls_to_rgb(int *pr, int *pg, int *pb, int ih, int il, int is)
/*****************************************************************************
 *
 ****************************************************************************/
{
	long	m1, m2;
	SHORT	h,l,s;
	SHORT	rv, gv, bv;

	h = ih;
	l = il;
	s = is;

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
		s = hlsrgb_value(m1, m2, h - 256/3);
		l = hlsrgb_value(m1, m2, h);
		h = hlsrgb_value(m1, m2, h + 256/3);
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

	*pr = rv;
	*pg = gv;
	*pb = bv;
}

void rgb_to_hls(int ir, int ig, int ib, int *ph, int *pl, int *ps)
/*****************************************************************************
 * routine copped from p618 of Foley and Van Dam Fundamentals of Interactive
 * computer graphics.  Converted to integer math by Jim Kent.
 ****************************************************************************/
{
	SHORT max,min;
	SHORT rc,gc,bc;
	SHORT r,g,b;
	SHORT hv, lv, sv;

	r = ir;
	g = ig;
	b = ib;

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

	*ph = hv;
	*pl = lv;
	*ps = sv;
}
