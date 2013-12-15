/* gldcircl.c - Simple stepping draw a circle algorithm.  
	Don't even correct aspect ratio. */

#include "gfx.h"


/* tables for small circles y,x0,x1,width */

static BYTE c2tab[] = {
	-1,-1,0,2,
	 0,-1,0,2,
};
static BYTE c10tab[] = {
	-5,-1,0,2,
	-4,-3,2,2,
	-3,-4,3,1,
	-2,-4,3,1,
	-1,-5,4,1,
	 0,-5,4,1,
	 1,-4,3,1,
	 2,-4,3,1,
	 3,-3,2,2,
	 4,-1,0,2,
};

static BYTE c11tab[] = {
	-5,-1,1,3,
	-4,-3,3,2,
	-3,-4,4,1,
	-2,-4,4,1,
	-1,-5,5,1,
	 0,-5,5,1,
	 1,-5,5,1,
	 2,-4,4,1,
	 3,-4,4,1,
	 4,-3,3,2,
	 5,-1,1,3,
};

static BYTE c12tab[] = {
	-6,-2,1,4,
	-5,-4,3,2,
	-4,-5,4,1,
	-3,-5,4,1,
	-2,-6,5,1,
	-1,-6,5,1,
	 0,-6,5,1,
	 1,-6,5,1,
	 2,-5,4,1,
	 3,-5,4,1,
	 4,-4,3,2,
	 5,-2,1,4,
};

static BYTE c13tab[] = {
	-6,-1,1,3,
	-5,-3,3,2,
	-4,-4,4,1,
	-3,-5,5,1,
	-2,-5,5,1,
	-1,-6,6,1,
	 0,-6,6,1,
	 1,-6,6,1,
	 2,-5,5,1,
	 3,-5,5,1,
	 4,-4,4,1,
	 5,-3,3,2,
	 6,-1,1,3,
};

Errcode dcircle(SHORT xcen, SHORT ycen, SHORT diam, 
	VFUNC dotout, void *dotdat, EFUNC hlineout, void *hlinedat, Boolean filled)



/* Note this function doesn't draw the same dot or same horizontal line
   twice so that the marqi and gel respectively will work */
{
int ret;		/* error code return passed back from hlineout */
register int x;
register BYTE *tab;
int width, x2;

	switch(diam)
	{
		case 2:
			tab = c2tab;
			break;
		case 10:
			tab = c10tab;
			break;
		case 11:
			tab = c11tab;
			break;
		case 12:
			tab = c12tab;
			break;
		case 13:
			tab = c13tab;
			break;
		default:
			return(doval(xcen,ycen,diam,1,1,
						 dotout,dotdat,hlineout,hlinedat,filled));
	}

	if(filled)
	{
		while(diam--)
		{
			if ((ret = (*hlineout)(ycen + tab[0], 
								   xcen + tab[1],
								   xcen + tab[2],hlinedat)) != 0)
			{
				return(ret);
			}
			tab += 4;
		}
	}
	else
	{
		width = tab[3];
		x = xcen+tab[1];
		while(width-- > 0)
			(*dotout)(x++,ycen+tab[0],dotdat);

		tab += 4;
		--diam;

		while(--diam)
		{
			x = xcen + tab[1];
			x2 = xcen + tab[2];
			width = tab[3];
			while(width-- > 0)
			{
				(*dotout)(x++,ycen+tab[0],dotdat);
				(*dotout)(x2--,ycen+tab[0],dotdat);
			}
			tab += 4;
		}
		width = tab[3];
		x = xcen+tab[1];
		while(width-- > 0)
			(*dotout)(x++,ycen+tab[0],dotdat);
	}
	return(0);
}

