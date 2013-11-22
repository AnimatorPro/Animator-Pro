/* tablet.c - stuff to send and recieve things from serial port for 
	digitizing tablets */

#include "jimk.h"

to_tablet(c)
UBYTE c;
{
union regs r;

r.w.dx = vconfg.comm_port;
r.b.ah = 1;
r.b.al = c;
sysint(0x14,&r,&r);
if ((r.b.ah & 0x80) ==  0)
	return(1);
else
	return(0);
}

read_tablet(c, count)
UBYTE *c;
int count;
{
union regs r;

while (--count >= 0)
	{
	r.w.dx = vconfg.comm_port;
	r.b.ah = 2;
	sysint(0x14,&r,&r);
	if ((r.b.ah & 0x80) !=  0)
		return(0);
	*c++ = r.b.al;
	}
return(1);
}

comm_ready()
{
union regs r;

r.w.dx = vconfg.comm_port;
r.b.ah = 3;
sysint(0x14,&r,&r);
return(r.b.ah&1);
}

flush_comm()
{
char dummy;
int i;

while (comm_ready())
	{
	if (!read_tablet(&dummy, 1))
		return;
	}
}

/* Wait 10 vertical blanks */
wait_10milli()
{
int i;

i = 11;
while (--i >= 0)
	wait_sync();	
}


