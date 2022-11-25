#include "util.h"

ULONG str_crcsum(char *buf)
{
register ULONG crcsum;
register ULONG op;

	if(!buf)
		return(0);

	crcsum = 0;
	op = 0;

	/* add value to crcsum */

    while(*buf)
	{
       	crcsum += (ULONG)(*buf++) << op;
       	if(++op == 25) 
			op = 0;
	}
  	return(crcsum);
}
