#ifndef TEST_H
#define TEST_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef PTRMACRO_H
	#include "ptrmacro.h"
#endif

extern Errcode init_pj_startup();

#define to_be_bracketed(a,b,c,d,e) WBRACK(to_be_bracketed,(a,b,c,d,e))

#define widge_query(s)  (*((LFUNC)(widge_funcs->testdat)))(s)


#endif
