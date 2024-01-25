#ifndef MATHHOST_H
#define MATHHOST_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef REXLIB_H 
	#include "rexlib.h" 
#endif

typedef struct mathhost_lib
	{
	Libhead hdr;
	double	(*acos)( double x );
	double	(*asin)( double x );
	double	(*atan)( double x );
	double	(*atan2)( double y, double x );
	double	(*ceil)( double x );
	double	(*cos)( double x );
	double	(*cosh)( double x );
	double	(*exp)( double x );
	double	(*fabs)( double x );
	double	(*floor)( double x );
	double	(*fmod)( double x, double y );
	double	(*frexp)( double value, int *exp );
	double	(*ldexp)( double x, int exp );
	double	(*log)( double x );
	double	(*log10)( double x );
	double	(*modf)( double value, double *iptr );
	double	(*pow)( double x, double y );
	double	(*sin)( double x );
	double	(*sinh)( double x );
	double	(*sqrt)( double x );
	double	(*tan)( double x );
	double	(*tanh)( double x );
} Mathhost_lib;

#endif /* AASYSLIB_H */
