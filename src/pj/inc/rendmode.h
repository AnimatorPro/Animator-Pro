#ifndef RENDMODE_H
#define RENDMODE_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

typedef struct rendmode {
	SHORT color;			/* current drawing color */
	BYTE under;  			/* under mode ie only draw where destination is
							 * tcolor */
	BYTE one_color;			/* render rasters as one color mask */
	BYTE fit_colors;

} Rendmode;


#endif /* RENDMODE_H */
