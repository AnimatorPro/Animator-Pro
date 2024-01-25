#include "stdtypes.h"
#include "regs.h"

Boolean pj_comm_exists(int port)

/* initialization of port to 9600 baud odd parity 1 stop 8 bit data... */
/* from Norton's Programmers Guide to IBM PC & PS2 p 228 */
/* 111			buad rate 9600
      01		odd parity
	    0		1 stop bit
		 11		8 bit data
		    = 11101011 = 0xeb */
{
union abcd_regs r;

	r.w.dx = port;
	r.b.al = 0xeb;
	r.b.ah = 0;
	jcomm(&r);
	return(r.b.ah != 0);
}
