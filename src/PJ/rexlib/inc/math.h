#ifndef MATH_H
#define MATH_H

#ifndef COMPILER_H
	#include "compiler.h"
#endif


#ifdef __WATCOMC__

	#pragma aux PRAGMA_FLOAT "*" parm caller [] value no8087;

	#pragma aux (PRAGMA_FLOAT) fabs;

	#ifdef	__INLINE_FUNCTIONS__  /* a watcom internal define !!! */
		 double  fabs( double __x );
		 #define fabs(x) _inline_fabs(x)
	#endif

	#pragma aux (PRAGMA_FLOAT) _r_ml_tanh;
	#pragma aux (PRAGMA_FLOAT) _r_ml_sinh;
	#pragma aux (PRAGMA_FLOAT) _r_ml_floor;
	#pragma aux (PRAGMA_FLOAT) _r_ml_cosh;
	#pragma aux (PRAGMA_FLOAT) _r_ml_ceil;
	#pragma aux (PRAGMA_FLOAT) acos;
	#pragma aux (PRAGMA_FLOAT) asin;
	#pragma aux (PRAGMA_FLOAT) atan;
	#pragma aux (PRAGMA_FLOAT) atan2;
	#pragma aux (PRAGMA_FLOAT) ceil;
	#pragma aux (PRAGMA_FLOAT) cos;
	#pragma aux (PRAGMA_FLOAT) cosh;
	#pragma aux (PRAGMA_FLOAT) exp;
	#pragma aux (PRAGMA_FLOAT) fabs;
	#pragma aux (PRAGMA_FLOAT) floor;
	#pragma aux (PRAGMA_FLOAT) fmod;
	#pragma aux (PRAGMA_FLOAT) frexp;
	#pragma aux (PRAGMA_FLOAT) ldexp;
	#pragma aux (PRAGMA_FLOAT) log;
	#pragma aux (PRAGMA_FLOAT) log10;
	#pragma aux (PRAGMA_FLOAT) modf;
	#pragma aux (PRAGMA_FLOAT) pow;
	#pragma aux (PRAGMA_FLOAT) sin;
	#pragma aux (PRAGMA_FLOAT) sinh;
	#pragma aux (PRAGMA_FLOAT) sqrt;
	#pragma aux (PRAGMA_FLOAT) tan;
	#pragma aux (PRAGMA_FLOAT) tanh;

	#pragma aux _a_a_mathlib "*";

#endif /* __WATCOMC__ */

/* symbol redefinitions so any compiler internal recognition of them is 
 * voided both _r_ml_XX() and XX() are actually present for every call */

#ifndef fabs
	#define	fabs _r_ml_fabs
#endif

/* these calls are not in the emulator and we don't want any compilers 
 * dragging in any private in line code not handled by the emulator */

#define tanh _r_ml_tanh
#define sinh _r_ml_sinh
#define floor _r_ml_floor
#define cosh _r_ml_cosh
#define ceil _r_ml_ceil

double	acos( double __x );
double	asin( double __x );
double	atan( double __x );
double	atan2( double __y, double __x );
double	ceil( double __x );
double	cos( double __x );
double	cosh( double __x );
double	exp( double __x );
double	fabs( double __x );
double	floor( double __x );
double	fmod( double __x, double __y );
double	frexp( double __value, int *__exp );
double	ldexp( double __x, int __exp );
double	log( double __x );
double	log10( double __x );
double	modf( double __value, double *__iptr );
double	pow( double __x, double __y );
double	sin( double __x );
double	sinh( double __x );
double	sqrt( double __x );
double	tan( double __x );
double	tanh( double __x );


#endif /* MATH_H */
