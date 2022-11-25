/*****************************************************************************
 * FLIERROR.C - Error reporting for flic read/write routines.
 *
 * NOTE!!!
 *
 *	This module is a temporary kludge!	When compiled on the pj side, this
 *	will pass a message and the flicfile name to the standard errline()
 *	routine so that it gets reported to the user.  When compiled as part of
 *	the flilib code, it just returns the error code without direct reporting,
 *	because errline() is not implemented on the flilib side.
 *
 *	The eventual fix will be to change the pj side to handle error reporting
 *	at a higher level, with the flic read/write routines just returning an
 *	error status and not doing any direct error reporting.	This fix is being
 *	deferred for the time being, due to complexity and time constraints.
 *
 *	The implication here is that clients of the flilib code MUST NOT count
 *	on the continued existance of this routine, and must not 'hook' this
 *	reporting point by defining their own pj_fli_error_report() function.
 *	This function will definately cease to exist in the near future, and
 *	flic routines which currently handle errors by doing a
 *		return pj_fli_error_report(err, msg, file);
 *	will in the future be using a simple
 *		return err;
 *	statement instead.	You have been warned!
 *
 *	The FLILIB_CODE macro is automatically defined via a -D command line
 *	switch in the MAKE.INC file for the flilib code.  It will never be
 *	defined when compiling as a part of PJ, CONVERT, PLAYER, et. al.
 *
 *	When the pj side is fixed, and this module becomes obsolete, don't
 *	forget to nuke the prototype for it in FLI.H!
 ****************************************************************************/

#include "fli.h"

extern Errcode errline(Errcode err, char *fmt, ...);

Errcode pj_fli_error_report(Errcode err, char *msg, char *filename)
{

#ifndef FLILIB_CODE

	errline(err, msg, filename);

#endif

	return err;
}
