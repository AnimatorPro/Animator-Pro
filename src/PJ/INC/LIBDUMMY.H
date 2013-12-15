#ifndef LIBDUMMY_H
#define LIBDUMMY_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

/* dummy do nothing functions for libraries */

void pj_vdo_nutin(void);
SHORT pj_sdo_nutin(void);
Pixel pj_rcdo_nutin(void);
Errcode pj_errdo_unimpl(void);
Errcode pj_errdo_success(void);

#endif /* LIBDUMMY_H */
