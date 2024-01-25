#ifndef TWEENMAG_H
#define TWEENMAG_H

#ifndef STDTYPES_H
#include "stdtypes.h"
#endif

struct poly;

#define MAG_MAGNET 0
#define MAG_BLOW 1

extern Errcode
mag_loop(struct poly *poly, Pixel color, long min_dist, int mag_mode);

#endif
