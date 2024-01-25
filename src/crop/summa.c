
/* summa.c - Fetch umouse_x, mouse_buttons etc from summa graphics
   12x12 Summa Sketch tablet.  Basically got to yammer out the
   serial port at it.  */

#include "jimk.h"

static
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

static
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

static 
comm_ready()
{
union regs r;

r.w.dx = vconfg.comm_port;
r.b.ah = 3;
sysint(0x14,&r,&r);
return(r.b.ah&1);
}

static
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

static
wait_10milli()
{
wait_sync();
wait_sync();
}

static char got_summa;

setup_summa()
{
to_tablet(0x20);	/* sync up autobaud */
to_tablet(0);		/* reset */
flush_comm();
wait_10milli();
to_tablet(0x44);	/* remote request mode */
to_tablet(0x62);    /* coordinates from upper left */
to_tablet(0x11);    /* let her rip */
wait_10milli();
}

/* initialization of port to 9600 baud odd parity 1 stop 8 bit data... */
/* from Norton's Programmers Guide to IBM PC & PS2 p 228 */
/* 111			buad rate 9600
      01		odd parity
	    0		1 stop bit
		 11		8 bit data
		    = 11101011 = 0xeb */
init_summa()
{
union regs r;

/* set things up to the right baud rate */
r.w.dx = vconfg.comm_port;
r.b.al = 0xeb;
r.b.ah = 0;
sysint(0x14, &r, &r);
setup_summa();
got_summa = 1;
return(1);
}

sscale_by(x,p,q)
int x,p,q;
{
long l;

l = x;
l *= p;
l /= q;
return(l);
}

c_summa()
{
extern WORD umouse_x, umouse_y;
UBYTE rbuf[6];

to_tablet(0x50);	/* hey babe, where's the cursor? */
if (read_tablet(rbuf, 5) )
	{
	if (rbuf[0] & 0x80)
		{
		mouse_button = rbuf[0] & 0x3;
		if (vconfg.pucky4)	/* puck??? reverse buttons. */
			mouse_button = ((mouse_button&1)<<1)+((mouse_button&2)>>1);
		umouse_x = rbuf[1] + (rbuf[2]<<7);
		umouse_y = rbuf[3] + (rbuf[4]<<7);
		umouse_x >>= 2;
		umouse_y = sscale_by(umouse_y, 220, 1000);
		umouse_x -= 30;
		umouse_y -= 40;
		}
	else	/* out of phase */
		{
		setup_summa();
		}
	}
else
	setup_summa();
}


cleanup_summa()
{
if (got_summa)
	{
	to_tablet(0x13);	/* SHADDUP already... */
	}
}

