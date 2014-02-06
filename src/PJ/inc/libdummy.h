#ifndef LIBDUMMY_H
#define LIBDUMMY_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

struct rastlib;

/* dummy do nothing functions for libraries */

void pj_vdo_nutin(void);
SHORT pj_sdo_nutin(void);
Pixel pj_rcdo_nutin(void);
Errcode pj_errdo_unimpl(void);
Errcode pj_errdo_success(void);

extern void pj_init_null_rastlib(struct rastlib *lib);
extern void *pj_get_null_lib(void);

#endif /* LIBDUMMY_H */
