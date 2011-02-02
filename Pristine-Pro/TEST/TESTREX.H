#ifndef TESTREX_H
#define TESTREX_H

#ifndef REXLIB_H
	#include "rexlib.h"
#endif

#define TEST_DRIVER 1000

typedef struct testrex {
	Rexlib hdr; /* TEST_DRIVER, 0 */
	int (*dotest)();
	int (*printf)(char *f,...);
	long (*clock_1000)(void);
} Testrex;

#endif /* TESTREX_H */
