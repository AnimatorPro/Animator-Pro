#include "procblit.h"
#include "makehdr.c"

main(int argv,char **argc)
{
	openit(argv,argc);
	soset(Tcolxldat,TCX_TCOLOR, tcolor);
	soset(Tcolxldat,TCX_XLAT, xlat);
	closeit();
}
