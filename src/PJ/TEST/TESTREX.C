#include "testrex.h"

extern printf(char *fmt,...);
extern long pj_clock_1000();

test()
{
Errcode err;
Testrex *tr;
static Libhead *test_libs[] = {
	&aa_syslib,
	NULL
	};


	if((err = pj_rexlib_load("\\paa\\test\\test.rex", 
							  TEST_DRIVER, (Rexlib **)(&tr),
					   		  test_libs, NULL)) < Success)
	{
		errline(err,"error opening rexlib");
		exit(1);
	}
	tr->printf = printf;
	tr->clock_1000 = pj_clock_1000;

	(*(tr->dotest))();

	pj_rexlib_free((Rexlib **)(&tr));
}
