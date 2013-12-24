#include <stdio.h>
#include "pjbasics.h"

void main(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
	int 	i;
	ULONG	clock_ms;
	ULONG	clock_jiffies;
	ULONG	conv_ms;

	printf("%d millisecs is %d jiffies\n\n",
		DEFAULT_AINFO_SPEED,
		pj_clock_ms2jiffies(DEFAULT_AINFO_SPEED)
		   );

	pj_clock_init();

	printf("Comparing millisecond clock to jiffy clock...\n");

	for (i = 0; i < 100; ++i) {
		clock_ms	  = pj_clock_1000();
		clock_jiffies = pj_clock_ms2jiffies(clock_ms);
		conv_ms 	  = pj_clock_jiffies2ms(clock_jiffies);
		printf("Clock m=%08lx, clock j=%08lx, conv m=%08lx, conv delta=%ld\n",
			clock_ms, clock_jiffies, conv_ms, clock_ms-conv_ms);
	}

	pj_clock_cleanup();
}
