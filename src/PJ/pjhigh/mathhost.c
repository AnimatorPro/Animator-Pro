#define REXLIB_INTERNALS
#include "mathhost.h"
#include <math.h>


double _f_o_rce_reference(double in)
/* needed to force at least one reference to the emulator and cause it to
 * initialize the library properly so the rex library will have it all */
{
	return(sin(in));
}

Mathhost_lib aa_mathlib = {
	/* header */
	{
		sizeof(Mathhost_lib),
		AA_MATHLIB, AA_MATHLIB_VERSION,
	},
	acos,
	asin,
	atan,
	atan2,
	ceil,
	cos,
	cosh,
	exp,
	fabs,
	floor,
	fmod,
	frexp,
	ldexp,
	log,
	log10,
	modf,
	pow,
	sin,
	sinh,
	sqrt,
	tan,
	tanh,
};

/* register protocall calls in watcom library */

#ifdef NOTYET /* __FPI__  floating point instructions (watcom emulator calls) */
	__ACOS,
	__ASIN,
	__ATAN,
	__ATAN2,
	__COS,
	__COSH,
	__EXP,
	__FMOD,
	__LOG,
	__LOG10,
	__POW,
	__SIN,
	__SINH,
	__SQRT,
	__TAN,
	__TANH,
#endif /* not __FPI__ */


