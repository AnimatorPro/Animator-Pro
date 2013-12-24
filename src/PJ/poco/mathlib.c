
#include "pocolib.h"
#include <math.h>
#include <signal.h>
#include "errcodes.h"
#include "ptrmacro.h"

extern Errcode builtin_err;

static Lib_proto mathlib[] = {
/* Most of the ansi math library (not bits that use pointers) */
{acos, "double  acos(double x);"},
{asin, "double  asin(double x);"},
{atan, "double  atan(double x);"},
{atan2, "double  atan2(double y, double x);"},
{ceil, "double  ceil(double x);"},
{cos, "double  cos(double x);"},
{cosh, "double  cosh(double x);"},
{exp, "double  exp(double x);"},
{fabs, "double  fabs(double x);"},
{floor, "double  floor(double x);"},
{fmod, "double  fmod(double x, double y);"},
{log, "double  log(double x);"},
{log10, "double  log10(double x);"},
{pow, "double  pow(double x, double y);"},
{sin, "double  sin(double x);"},
{sinh, "double  sinh(double x);"},
{sqrt, "double  sqrt(double x);"},
{tan, "double  tan(double x);"},
{tanh, "double  tanh(double x);"},
};

//int matherr(struct exception *err_info)
///*****************************************************************************
// *
// ****************************************************************************/
//{
//	  builtin_err = Err_float;
//
//	  errline(Err_float, "%s(%f ...) code %d", err_info->name, err_info->arg1,
//		  err_info->type);
//	  raise(SIGFPE);
//	  return(1);
//}

Poco_lib po_math_lib = {
	NULL, "(C Standard) Math",
	mathlib, Array_els(mathlib),
	};
