/* gldoval.c - draws an oval, that is an ellipse with the axis alligned
 * horizontally or vertically.  (No tilted ellipses for us.) */

#include "gfx.h"

Errcode
doval(SHORT xcen, SHORT ycen, SHORT xdiam, SHORT xaspect, SHORT yaspect,
		dotout_func dotout, void *dotdat,
		hline_func hlineout, void *hlinedat,
		Boolean filled)
/* Draw an oval.   Diameter is in terms of x.  The y diameter is
 * 		ydiam = xdiam*yaspect/xaspect.
 * Note this function doesn't draw the same dot or same horizontal line
 * twice so that the marqi and gel respectively will work */
{
int ret;		/* error code return passed back from hlineout */
int err;
int derr, yerr, xerr;
int aderr, ayerr, axerr;
register int x,y;
int lasty;
int even1;

	xaspect *= xaspect;		/* stepper uses the square of the aspect ratio */
	yaspect *= yaspect;
	err = 0;
	even1 = (xdiam & 1)^1;
	x = xdiam>>1;
	lasty = y = 0;

	if(!even1 || !xdiam)
	{
		if(filled)
		{
			if ((ret = (*hlineout)(ycen,xcen-x,xcen+x,hlinedat)) != 0)
				return(ret);
			if (x <= 0)
				return(0);
		}
		else
		{
			(*dotout)(xcen-x,ycen,dotdat);
			if(x <= 0)
				return(0);
			(*dotout)(xcen+x,ycen,dotdat);
		}
	}

	for (;;)
	{
		axerr = xerr = err + yaspect*(-x-x+1);
		ayerr = yerr = err + xaspect*(+y+y+1);
		aderr = derr = yerr+xerr-err;
		if (aderr < 0)
			aderr = -aderr;
		if (ayerr < 0)
			ayerr = -ayerr;
		if (axerr < 0)
			axerr = -axerr;
		if (aderr <= ayerr && aderr <= axerr)
		{
			err = derr;
			x -= 1;
			y += 1;
		}
		else if (ayerr <= axerr)
		{
			err = yerr;
			y += 1;
		}
		else
		{
			err = xerr;
			x -= 1;
		}
		if (filled)
		{
			if(lasty != y)
			{
				if ((ret = (*hlineout)(ycen-y, xcen-x, 
									   xcen+x-even1,hlinedat)) != 0
					|| (ret = (*hlineout)(ycen+y-even1, 
										  xcen-x, xcen+x-even1,hlinedat)) != 0)
				{
					return(ret);
				}
				lasty = y;
			}
			if (x <= 0)
				break;
		}
		else
		{
			/* draw 4 quadrandts of a circle */

			(*dotout)(xcen+x-even1,ycen+y-even1,dotdat);
			(*dotout)(xcen+x-even1,ycen-y,dotdat);

			if(!even1 && x <= 0)
				break;

			(*dotout)(xcen-x,ycen+y-even1,dotdat);
			(*dotout)(xcen-x,ycen-y,dotdat);

			if(even1 && x <= 1)
				break;
		}
	}
	return(0);
}

