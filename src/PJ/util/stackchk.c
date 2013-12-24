#include "memory.h"

void pj_init_stack()

/* stuff all unused stack with cookies for later checking */
{
char stack[1];
char *slow;

	slow = _STACKLOW;
	/* fill stack with lots of cookies */

	/* 36 bytes less for what stuff bytes puts on and this function */
	pj_stuff_bytes(STACK_COOKIES, slow, (stack - slow) - 36);
}
int pj_get_stack_used()
{
int ssize;
char *slow;

	slow = _STACKLOW;
	ssize = _STACKTOP - slow;

	if(*slow == STACK_COOKIES)
		ssize -= pj_bsame(slow, ssize);
	return(ssize);
}

