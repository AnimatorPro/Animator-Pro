
#ifndef REGS_H
#define REGS_H

#ifndef STDTYPES_H
#include "stdtypes.h"
#endif /* STDTYPES_H */


struct babcd_regs
	{
	UBYTE al,ah,bl,bh,cl,ch,dl,dh;
	};

struct wabcd_regs
	{
	USHORT ax,bx,cx,dx;
	};

union abcd_regs
	{
	struct babcd_regs b;
	struct wabcd_regs w;
	};

#endif /* REGS_H */
