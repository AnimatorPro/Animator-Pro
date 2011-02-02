#include <signal.h>
#include "stdtypes.h"

test(int argc, char *argv[])
{
int i;
int dev_count;
UBYTE devs[26];

printf("About to get info\n");
for (i=2; i<26; ++i)
	{
	printf("%c: %d\n", i+'A', drive_info(i));
	}
#ifdef LATER
printf("About to get devices\n");
dev_count = pj_get_devices(devs);
printf("Got %d devices\n", dev_count);
for (i=0; i<dev_count; ++i)
	{
	printf("%c: ", devs[i] + 'A');
	}
printf("\n");
#endif 
}

