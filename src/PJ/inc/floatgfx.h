#ifndef FLOATGFX_H
#define FLOATGFX_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef VERTICES_H
	#include "vertices.h"
#endif

#ifndef FPMATH_H
	#include "fpmath.h"
#endif

#define ITHETA_TOFLOAT(ia,i2pi) (((double)ia)*(PI*2.0)/((double)i2pi))
#define FLOAT_TOITHETA(fa,i2pi) \
	((int)((fa*((double)i2pi)/(PI*2.0))+(angle<0?-0.5:0.5)))

double itheta_tofloat(int int_angle, int int_twopi);
int float_toitheta(double float_angle, int int_twopi);

double clipto_pi(double theta);
double clipto_2pi(double theta);

void frotate_points2d(double theta,
					 Short_xy *cent,
					 Short_xy *spt,
					 Short_xy *dpt,int count);


#endif /* FLOATGFX_H */
