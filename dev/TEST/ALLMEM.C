/* This program allocates all available memory 100K at a time and
 * prints out what it got, and then quits.
 */

#include <stdio.h>

void main()
{
int i;

for (i=0; ;++i)
	{
	if (malloc(1024*100) == NULL)
		break;
	}
printf("Allocated %ld bytes in %d blocks\n", i * 1024L * 100, i);
}
