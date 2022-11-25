#include "imath.h"

static int igcd(int a, int b)
/* find greatest common divisor of a and b using Euclid's algorithm. */
{
int swap;
int remainder;

if (a < b)		/* Force a >= b */
	{
	swap = a;
	a = b;
	b = swap;
	}
for (;;)
	{
	if ((remainder = a%b) == 0)
		return(b);
	else
		{
		a = b;
		b = remainder;
		}
	}
}


int ilcm(int a, int b)
/* find least common multiple */
{
a /= igcd(a,b);		/* divide one term by greatest common divisor */
return(a*b);
}
