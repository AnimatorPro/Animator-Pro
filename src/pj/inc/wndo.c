#include "wndo.h"
#include "makehdr.c"

#define prtf(name,f) soset(Wndo,name,f);

main(int argc,char **argv)
{
	openit(argc,argv);
	prtf(W_RASTS, rasts);
	prtf(W_BEHIND, behind);
	prtf(W_ONERAST, onerast);
	prtf(W_YDOTS, ydots);
	closeit();
}
