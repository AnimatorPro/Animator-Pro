#ifdef SLUFFED
#include "input.h"

SHORT toupper_inkey()
{
	return((icb.inkey&0xFF00)|toupper(icb.inkey&0x00FF));
}
#endif /* SLUFFED */
