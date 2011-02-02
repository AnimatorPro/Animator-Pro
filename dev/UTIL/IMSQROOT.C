/* sqrroot.c - use a bit-wise algorithm Robert Leyland certainly must
   understand better than I. */

/*
** Sqr_root() - Return the integer square root the the long integer i
*/

int sqr_root(long i)
{
unsigned long	mask;
long t;
unsigned short	result;
long		lolly;

/*
** Approximate starting mask value
*/

for (mask = 1, lolly = i; mask < lolly; mask <<= 1, lolly >>= 1)
;

result = 0;

while (mask)
{
t = result | mask;
if ((t * t) <= i)
    result = t;
mask >>= 1;
}

return ((int)result);
}
