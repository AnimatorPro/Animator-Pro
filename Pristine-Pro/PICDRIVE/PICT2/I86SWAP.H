#ifndef I86SWAP_H
#define I86SWAP_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef PTRMACRO_H
	#include "ptrmacro.h"
#endif


USHORT intel_swap_word(USHORT);
ULONG intel_swap_long(ULONG);
void *intel_swap_words(void *, int ct);
void *intel_swap_longs(void *, int ct);

#define ISET_OFFSET 0x4000
#define ISWAP_LONGS 0x8000

#define SWAP_LONGS(ct) (ISWAP_LONGS|ct)
#define SWAP_WORDS(ct) (ct)
#define SET_OFFSET(s,f) (ISET_OFFSET|OFFSET(s,f))
#define END_SWAPTAB  0


void intel_swap_struct(void *struc, USHORT *stab);


#endif /* I86SWAP_H Leave at end of file */
