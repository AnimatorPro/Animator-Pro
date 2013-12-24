#include "stdtypes.h"
#include "memory.h"
#include "poly.h"
#include "errcodes.h"

Errcode filled_polygon(Poly *poly,
	EFUNC hline, void *hldat, VFUNC line, void *ldat)
{
Errcode err;
	if((err = fill_poly_inside(poly,hline,hldat)) >= Success)
		hollow_polygon(poly,line,ldat,TRUE);
	return(err);
}

