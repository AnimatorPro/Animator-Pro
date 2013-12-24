#include "stdtypes.h"
#include <string.h>

Boolean pj_valid_suffix(char *suff)
/* if it is a valid file suffix containing valid characters may or may not
 * have a leading '.' */
{
int len;

	if(suff[0] == '.')
		++suff;
	len = strcspn(suff,".\"/[]|<>+=,?*");
	return((len < 4) && (suff[len] == 0));
}
