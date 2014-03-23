/* bhash.h - a way of caching calls to closestc to make it a little
 * faster to convert a bunch of true-color values to their closest
 * match in the current pencel color map. */
#ifndef BHASH_H
#define BHASH_H

#ifndef STDTYPES_H
#include "stdtypes.h"
#endif

struct rgb3;

Errcode make_bhash(void);
void free_bhash(void);
Boolean is_bhash(void);
int bclosest_col(const struct rgb3 *rgb, int count, SHORT dither);

#endif
