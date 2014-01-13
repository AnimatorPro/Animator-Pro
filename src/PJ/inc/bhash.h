/* bhash.h - a way of caching calls to closestc to make it a little
 * faster to convert a bunch of true-color values to their closest
 * match in the current pencel color map. */
#ifndef BHASH_H
#define BHASH_H

#ifndef CMAP_H
	#include "cmap.h"
#endif

#ifdef SLUFFED
/* General true-color hash */
struct bhash {
	UBYTE valid;
	Rgb3 rgb;
	UBYTE closest;
};
#endif /* SLUFFED */

Errcode make_bhash(void);
void free_bhash(void);
Boolean is_bhash(void);
int bclosest_col(register Rgb3 *rgb,int count,SHORT dither);

#endif /* BHASH_H */
