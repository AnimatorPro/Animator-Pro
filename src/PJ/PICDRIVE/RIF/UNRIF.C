
/* unrif.c - medium to low level decompression routines for
   Amiga/Zoetrope rif format file.  (and ANIM op 5) */

#include "stdtypes.h"


UBYTE *decode_vcolumn(UBYTE *comp, UBYTE *plane, int BytesPerRow)
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

UBYTE *decode_vplane(UBYTE *comp, UBYTE *plane, int BytesPerRow)
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

UBYTE *decode_vkcolumn(UBYTE * comp, UBYTE * plane, 
					   int BytesPerRow, int *ytable)
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

UBYTE *decode_vkplane(UBYTE *comp, UBYTE *plane, 
					  int BytesPerRow, int *ytable)
{
int i;

i = BytesPerRow;
while (--i >= 0)
	{
	comp = decode_vkcolumn(comp, plane, BytesPerRow, ytable);
	plane += 1;
	}
return(comp);
}

