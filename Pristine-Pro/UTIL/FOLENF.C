#include "formatf.h"

fa_lenf(Formatarg *fa)
/* returns char length that a given formatf will give you */
{
	while(fa_getc(fa));
	return(fa->count - 1);
}
