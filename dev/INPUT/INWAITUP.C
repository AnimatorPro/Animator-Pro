
#define INPUT_INTERNALS
#include "input.h"

void wait_mbup(ULONG flags)
{
	flags &= (MBPEN|MBRIGHT);
	_poll_input(0);
	while(ISDOWN(flags))
		wait_input(MBPUP|MBRUP);
}
