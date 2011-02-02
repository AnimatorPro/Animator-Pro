#include "stdtypes.h"

/* trdummy.c dummy calls linked in with dos file library to allow it to be
 * used with initializer code that may also be used with temp files code */

Errcode set_temp_path(char *path)
{
	return(Success);
}
