#include "imath.h"

void calc_sieve(char *sieve, int max)
/* generate sieve table.  0's left on prime numbers */
{
int i,j;

for (j=2; j<max; j++)
	{
	if (sieve[j] == 0)
		{
		for (i=j+j; i<max; i+=j)
			{
			sieve[i] = 1;
			}
		}
	}
}
