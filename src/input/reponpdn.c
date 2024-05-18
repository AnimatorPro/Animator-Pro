#include "errcodes.h"
#include "input.h"

void repeat_on_pdn(void (*v)(void *data), void *data)
{
int i;
ULONG tout;

	tout = 500; /* 1/2 sec */
	for(i=0;;)
	{
		(*v)(data);
		if(!(ISDOWN(MBPEN))) 
			return;
		if((timed_wait_input(MBPUP,tout)) >= Success) /* timed out */
			return;
		if(tout > 100)
			tout = 100;
		else if(i >= 10)
		{
			tout = 40;
			continue;
		}
		++i;
	}
}
