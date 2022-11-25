/***************************************************************
Pict file pdr modules:

	Created by Peter Kennard.  Sept 29, 1991
		Implements non-pattern fill mode bitmap operations
		and transfer mode blit. Parses and ignores all other
		opcodes.  This file contains functions to convert basic data types
		from LSB->MSB to MSB->LSB byte ordering.
****************************************************************/
#include "stdtypes.h"
#include "ptrmacro.h"
#include "i86swap.h"

union lparts { ULONG l; USHORT w[2]; UBYTE b[4]; };

ULONG intel_swap_long(ULONG l)
/* Swap a long integer in 68000 format to intel byte order format
 * and return it. */
{
union lparts ol;
#define LP (*((union lparts *)&l))

	ol.b[0] = LP.b[3];
	ol.b[1] = LP.b[2];
	ol.b[2] = LP.b[1];
	ol.b[3] = LP.b[0];
	return(ol.l);
#undef LP
}
void *intel_swap_longs(void *l, int count)
/* Intel swap a bunch of 4 byte long words from 68000 format to intel format. */
{
UBYTE swaper;
#define LB ((UBYTE *)l)

	while(count-- > 0)
	{
		swaper = LB[0];
		LB[0] = LB[3];
		LB[3] = swaper;

		swaper = LB[2];
		LB[2] = LB[1];
		LB[1] = swaper; 
		l = OPTR(l,sizeof(ULONG));
	}
	return(l);
#undef LB
}
USHORT intel_swap_word(USHORT w)
/* Intel swap a 2 byte word. */
{
	return((w << 8) + (w >> 8));
}
void *intel_swap_words(void *pt, int count)
/* Intel swap a bunch of 2 byte words. */
{
register UBYTE *rpt = pt;
register UBYTE swap;

while (--count >= 0)
	{
	swap = rpt[1];
	rpt[1] = rpt[0];
	rpt[0] = swap;
	rpt += 2;
	}
	return(rpt);
}
void intel_swap_struct(void *struc, USHORT *stab)
/* Intel swaps a structure using a table of swap lengths for the fields.
 * See i86swap.h for the definition of the input table "stab". */
{
USHORT tab;
void *s;

	s = struc;

	while((tab = *stab) != 0)
	{
		tab = *stab++;
		if(tab & ISET_OFFSET)
		{
			s = OPTR(struc,(tab & (~ISET_OFFSET)));
		}
		else if(tab & ISWAP_LONGS)
		{
			s = intel_swap_longs(s,tab & (~ISWAP_LONGS));
		}
		else
		{
			s = intel_swap_words(s,tab);
		}
	}
}
