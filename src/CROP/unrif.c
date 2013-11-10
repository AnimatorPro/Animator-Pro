
/* unrif.c - medium to low level routines to read in an Amiga/Zoetrope
   rif format file.  See also readrif.c */

#include "jimk.h"

extern unsigned WORD ytable[YMAX];

#ifdef C_CODE
conv_screen(bits)
PLANEPTR bits;
{
int i;
PLANEPTR bytes;

bytes = (PLANEPTR)0xa0000000;
i = 8000;
while (--i >= 0)
	{
	conv8(bits[0*PLANE_SIZE],
		bits[1*PLANE_SIZE],
		bits[2*PLANE_SIZE],
		bits[3*PLANE_SIZE],
		bits[4*PLANE_SIZE], bytes);
	bits += 1;
	bytes += 8;
	}
}
#endif /* C_CODE */

#ifdef SLUFFED
conv8(a, b, c, d, e, out)
int a, b, c, d, e;
PLANEPTR out;
{
UBYTE byte;
int i;
int mask;

mask = 0x80;
i = 8;
while (--i >= 0)
	{
	byte = 0;
	if (a&mask)
		byte |= 1;
	if (b&mask)
		byte |= 2;
	if (c&mask)
		byte |= 4;
	if (d&mask)
		byte |= 8;
	if (e&mask)
		{
		byte |= 16;
		}
	*out++ = byte;
	mask >>= 1;
	}
}
#endif /* SLUFFED */


PLANEPTR 
decode_vcolumn(comp, plane, BytesPerRow)
PLANEPTR comp, plane;
WORD BytesPerRow;
{
int op_count;
BYTE op;
UBYTE data;

op_count = *comp++;
while (--op_count >= 0)
	{
	op = *comp++;
	if (op&0x80)
		{
		op &=0x7f;
		while (--op >= 0)
			{
			*plane = *comp++;
			plane += BytesPerRow;
			}
		}
	else	/* same op */
		{
		data = *comp++;
		while (--op >= 0)
			{
			*plane = data;
			plane += BytesPerRow;
			}
		}
	}
return(comp);
}

PLANEPTR
decode_vplane(comp, plane, BytesPerRow)
PLANEPTR comp, plane;
WORD BytesPerRow;
{
int i;

i = BytesPerRow;
while (--i >= 0)
	{
	comp = decode_vcolumn(comp, plane, BytesPerRow);
	plane += 1;
	}
return(comp);
}

PLANEPTR 
decode_vkcolumn(comp, plane, BytesPerRow)
PLANEPTR comp, plane;
WORD BytesPerRow;
{
int op_count;
BYTE op;
UBYTE data;

op_count = *comp++;
while (--op_count >= 0)
	{
	op = *comp++;
	if (op & 0x80)
		{
		op &= 0x7f;
		while (--op >= 0)
			{
			*plane = *comp++;
			plane += BytesPerRow;
			}
		}
	else if (op == 0)
		{
		op = *comp++;
		data = *comp++;
		while (--op >= 0)
			{
			*plane = data;
			plane += BytesPerRow;
			}
		}
	else
		{
		plane += ytable[op];
		}
	}
return(comp);
}

PLANEPTR
decode_vkplane(comp, plane, BytesPerRow)
PLANEPTR comp, plane;
WORD BytesPerRow;
{
int i;

i = BytesPerRow;
while (--i >= 0)
	{
	comp = decode_vkcolumn(comp, plane, BytesPerRow);
	plane += 1;
	}
return(comp);
}

