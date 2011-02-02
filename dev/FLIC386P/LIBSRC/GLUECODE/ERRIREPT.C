/*****************************************************************************
 * ERRIREPT.C - Stub module for internal error handling, just passes code thru.
 *
 *	The real usefullness of the pj_error_internal() function occurs when a
 *	version of it is coded in the client code and it gets used instead of
 *	this stub.	In that case, the client code can report the error message,
 *	the module that detected the error, and the line within that module.
 *	This can eliminate the need for the client code to check the return value
 *	from every silly little fliclib function call, yet still detect NULL
 *	pointers and parameter errors during development.  (After all, one expects
 *	that a pj_raster_clear(), for example, is not going to fail.  At least,
 *	not after any silly typos or other NULL-pointer causes are fixed during
 *	early development.)
 *
 ****************************************************************************/

#include "flicglue.h"

Errcode pj_error_internal(Errcode err, char *file, int line)
/*****************************************************************************
 * just return the error code, and ignore the other parms, in this stub.
 ****************************************************************************/
{
	return err;
}
