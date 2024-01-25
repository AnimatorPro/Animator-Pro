#include "stdio.h"
#include "errcodes.h"
#include "ffile.h"

Errcode ffile_error()
{
	return(pj_errno_errcode());
}
