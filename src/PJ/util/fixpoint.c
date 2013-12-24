#include "fixpoint.h"

fixpoint fixpoint_div(fixpoint a, fixpoint b)
{
	return(((a<<FIXPOINT_SHIFT)+b/2)/b);
}
