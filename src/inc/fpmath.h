#ifndef FPMATH_H
#define FPMATH_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef _MATH_H_INCLUDED
	#include <math.h>
#endif

int roundtoint(double fpval);
#define roundtolong(f) roundtoint(f)

#define PI (3.14159265359)

#define to_degrees(rad) ((360.0*((double)rad))/(PI*2))
#define to_radians(degrees) (((PI*2)*((double)degrees))/(360.0))


#endif /* FPMATH_H */
