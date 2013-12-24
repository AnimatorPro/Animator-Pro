#include "stdtypes.h"

ULONG mem_crcsum(void *sbuf, LONG size)
{
register UBYTE *buf;
register UBYTE *maxbuf;
register ULONG crcsum;
register ULONG op;

	buf = sbuf;
	maxbuf = buf + size;

	crcsum = 0;
	op = 0;

	/* add value to crcsum */

    while(buf < maxbuf)
	{
       	crcsum += (ULONG)(*buf++) << op;
       	if(++op == 25) 
			op = 0;
	}
  	return(crcsum);
}
