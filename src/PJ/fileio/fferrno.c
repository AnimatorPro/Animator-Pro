/* fferrno.c */

#ifdef USE_LFILE
	#include "lstdio.h"
#else
	#include <errno.h>
#endif
#include "ffile.h"

Errcode pj_errno_errcode(void)
{
	return errno;
}
