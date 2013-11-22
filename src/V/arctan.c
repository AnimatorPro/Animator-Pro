/* arctan.c - a relatively fast table based method for finding out an
   angle given the x,y coordinates of a ray at that angle */

#include "jimk.h"

/* arc-tangent table */
static
tan_tab[] = {
1,2,3,5,6,7,8,10,11,12,
13,15,16,17,18,19,21,22,23,24,
25,26,28,29,30,31,32,33,34,35,
36,37,38,39,40,41,42,43,44,45,
46,47,48,49,49,50,51,52,53,54,
54,55,56,57,57,58,59,59,60,61,
62,62,63,63,
66,70,74,78,81,84,86,89,91,93,
94,96,97,98,100,101,102,103,104,104,
105,106,107,107,108,108,109,109,110,110,
111,111,112,112,112,113,113,113,114,114,
114,114,115,115,115,115,116,116,116,116,
116,117,117,117,117,117,117,118,118,118,
118,118,118,118,
118,120,121,122,123,123,124,124,124,124,
125,125,125,125,125,125,126,126,126,126,
126,126,126,126,126,126,126,126,126,126,
126,126,126,126,126,126,126,127,127,127,
127,127,127,127,127,127,127,127,127,127,
127,127,127,127,127,127,127,127,127,127,
127,127,127,127,128,128} ;


static short small_arctan(x,y) 
/* Pass y,x, and an angle between 0 and 512 will be returned*/
long x,y; /* */
{
   register int index; /* calculated offset into arctan table */
   register int r;     /* temperary variable */
   register int x_neg,y_neg; /* sign flags */

   x_neg = y_neg = 0;

   if (x<0) { /* make x and y positive, but set flags to remember */
      x_neg = 1;
      x = -x;
   }
   if (y<0) {
      y_neg = 1;
      y = -y;
   }
   if (x == 0) 
   		r = 128;
   else if (y == 0)
   		r = 0;
   else
	   {
	   if (y <= x)
	      index = (y<<6)/x; /* table is divided into three parts, so figure */
	   else if (y/x < 9)    /* which part to use */
	      index = 56 + ((((y<<4)/x)-1)>>1);
	   else if ((r = (y/x)) <= 135)
	      index = 128+((r-9)>>1);
	   else 
	      index = 192; /* point index to max value */

	   r = tan_tab[index]; /* get theta from lookup table */
	   }

   if (!x_neg && !y_neg) /* the table only has the first quadrant, so use */
      return(r);         /* semetries to get the other quadrants */
   if (x_neg && !y_neg)
      return(256 - r);
   if (x_neg && y_neg)
      return(256 + r);
   return((512 - r));
} /* end arctan */


/* return angle of ray from center to x,y */
arctan(x,y)
int x,y;
{
return ( small_arctan((long)y,(long)x)<<1  );
}

/* return an angle normalized to be between -PI and PI */
arcnorm(t)
{
t += TWOPI/2;
t = -t;
return(t&(TWOPI-1));
}
