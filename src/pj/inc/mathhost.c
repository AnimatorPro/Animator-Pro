#include <stdio.h>
#include "mathhost.h"
#include "ptrmacro.h"
#include "makehdr.c"

char fmt1[] = "math_%s equ +0%xH\n";

#define moset(f) outf(fmt1,#f,OFFSET(Mathhost_lib,f))

main(int argc,char **argv)
{
	openit(argc,argv);
	moset(acos);
	moset(asin);
	moset(atan);
	moset(atan2);
	moset(ceil);
	moset(cos);
	moset(cosh);
	moset(exp);
	moset(fabs);
	moset(floor);
	moset(fmod);
	moset(frexp);
	moset(ldexp);
	moset(log);
	moset(log10);
	moset(modf);
	moset(pow);
	moset(sin);
	moset(sinh);
	moset(sqrt);
	moset(tan);
	moset(tanh);
	closeit();
}
