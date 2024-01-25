#ifndef RGBCMAP_H
#define RGBCMAP_H
#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif
#ifndef CMAP_H
	#include "cmap.h"
#endif

			/* This is the number of bytes in our color histagram,
			 * which contains one bit for each color in a 64x64x64 RGB space */

#define HIST_BYTES	  ((64*64*64)/8)
#define HIST64_SIZE   ((64*64*64)/8)
#define HIST256_SIZE  ((256*256*256)/8)

Errcode alloc_histogram(UBYTE **h);
void	freez_histogram(UBYTE **h);

void	hist_set_bits(UBYTE *h,  UBYTE **rgb_bufs, int count);
Errcode hist_to_cmap(UBYTE **phist, Cmap *cmap);

#endif /* RGBCMAP_H */
