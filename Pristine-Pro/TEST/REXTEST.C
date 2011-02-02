#include "testrex.h"

extern long pj_clock_1000();

test()
{
Errcode err;
Testrex *tr;
static Libhead *test_libs[] = {
	&aa_syslib,
	&aa_mathlib, 
	NULL
	};


	if((err = pj_rexlib_load("test.rex", TEST_DRIVER, (Rexlib **)(&tr),
					   			test_libs, NULL)) < Success)
	{
		printf("error opening rexlib %d\n", err);
		exit(1);
	
	}
	tr->printf = printf;
	tr->clock_1000 = pj_clock_1000;

	(*(tr->dotest))();

	pj_rexlib_free((Rexlib **)(&tr));
}
