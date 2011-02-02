/* ccache.c - This is the virtual color caching system for speeding up
 * finding which color map entry is the closest to a given RGB value.
 * Here we figure out which one of 3 schemes - bbhash64, bhash, or ghash
 * to use based on whether the picture is greyscale or color, and whether
 * 6 bits or 8 bits of the RGB components are significant.
 *
 * Here also we handle a very simple error propagation dithering.
 */

#include "ccache.h"

int cc_rerr,cc_gerr,cc_berr;

void cc_flush_dither_err(void)
{
	cc_rerr = cc_gerr = cc_berr = 0;
}

Errcode cc_make(Ccache **pcc, Boolean is_grey, UBYTE colors_256)
/* Figure out which cache scheme to use. */
{
Ccache *cc;
Errcode err;

if (is_grey)
	{
	cc = &cc_ghash;
	err = cc->make_ccache(cc);
	}
else
	{
	if (colors_256)
		cc = &cc_bhash;
	else
		cc = &cc_bbhash64;
	if ((err = cc->make_ccache(cc)) < Success)	/* not enough memory */
		{										/* So use simple ghash */
		cc = &cc_ghash;
		err = cc->make_ccache(cc);
		}
	}
cc_flush_dither_err();
cc->total = cc->misses = 0;
*pcc = cc;
return(err);
}

void cc_free(Ccache *cc)
{
if (cc != NULL)
	cc->free_ccache(cc);
}

int cc_closest(Ccache *cc, Rgb3 *rgb, Cmap *cmap, Boolean dither)
/* Find the closest color using the color cache */
{
Rgb3 drgb;
int r,g,b;
int temp;

++(cc->total);
if (dither)
	{
	temp = rgb->r + cc_rerr;
	if (temp < 0)
		temp = 0;
	if (temp > RGB_MAX-1)
		temp = RGB_MAX-1;
	drgb.r = r = temp;
	temp = rgb->g + cc_gerr;
	if (temp < 0)
		temp = 0;
	if (temp > RGB_MAX-1)
		temp = RGB_MAX-1;
	drgb.g = g = temp;
	temp = rgb->b + cc_berr;
	if (temp < 0)
		temp = 0;
	if (temp > RGB_MAX-1)
		temp = RGB_MAX-1;
	drgb.b = b = temp;
	rgb = &drgb;
	}
temp = cc->ccache_closest(cc,rgb,cmap);
if (dither)
	{
	rgb = cmap->ctab + temp;
	cc_rerr = 3*(r - rgb->r)/4;
	cc_gerr = 3*(g - rgb->g)/4;
	cc_berr = 3*(b - rgb->b)/4;
	}
return(temp);
}

