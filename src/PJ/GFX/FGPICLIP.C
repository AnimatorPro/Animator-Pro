#include "gfx.h"
#include "floatgfx.h"
#include "imath.h"

double clipto_pi(double theta)
/* clips a <-PI to >+PI angle a -PI to +PI angle */
{
	if(theta > PI)
		return(fmod(theta,PI*2) - PI*2);
	else if(theta < -PI)
		return(fmod(theta,PI*2) + PI*2);
	return(theta);
}
